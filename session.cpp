#include "session.h"
#include "exception.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "sender.h"
#include "receiver.h"

std::condition_variable inbox_cv;
std::mutex inbox_mutex;

volatile session_state_t Session::state;
std::vector<Message *> Session::inbox;

void Session::notify_incoming(Message *message)
{
  std::lock_guard<std::mutex> lg(inbox_mutex);

  if (message->code == CODE_MSG)
  {
    auto msg_message = dynamic_cast<MsgMessage *>(message);
    std::cout << msg_message->display_name << ": " << msg_message << std::endl;
  }

  inbox.push_back(message);

  inbox_cv.notify_one();
}

Session::Session(int _client_socket, unsigned int _max_retr, std::chrono::milliseconds _timeout)
{
  client_socket = _client_socket;
  std::cerr << "DEBUG: Created session. Server with " << _max_retr << " retr." << std::endl;
  timeout = _timeout;

  sender = new UDPSender(_client_socket, _max_retr, timeout, this);
  receiver = new UDPReceiver(client_socket, this);

  receiving_thread = std::jthread(UDPReceiver::receive, this, client_socket, sender);

  message_id = 1;
}

int Session::sendmsg(const std::string &_contents)
{
  if (state == STATE_START)
  {
    throw NotInChannel();
  }

  if (state != STATE_OPEN)
  {
    std::cout << "Not authenticated. Use /auth." << std::endl;
    std::cout << "DEBUG: state " << state << std::endl;
  }

  auto message = new MsgMessage(message_id, displayname, _contents);
  std::cout << "DEBUG: Sending message with contents" << " " << _contents << std::endl;
  sender->send_msg(message);

  message_id++;

  return 1;
}

int Session::join(const std::string &_channel_id, const std::string &_display_name)
{
  if (state == STATE_START)
  {
    throw NotAuthenticated();
  }

  MessageWithId *message = new JoinMessage(message_id, _channel_id, _display_name);
  sender->send_msg(message);

  state = STATE_JOIN;

  // TODO

  message_id++;

  return 0;
}

int Session::rename(const std::string &_new_name)
{
  throw NotImplemented();
}

session_state_t Session::get_state()
{
  return state;
}

int Session::auth(const std::string &_username, const std::string &_secret,
                  const std::string &_display_name)
{
  std::unique_lock ul(inbox_mutex);

  if (state != STATE_START)
  {
    std::cout << "Already authenticated." << std::endl;
  }

  username = _username;
  displayname = _display_name;
  secret = _secret;

  std::cerr << "DEBUG: sending AUTH." << std::endl;
  MessageWithId *message = new AuthMessage(username, secret, displayname, message_id);
  sender->send_msg(message);

  // Wait for REPLY
  state = STATE_AUTH;

  if(!inbox_cv.wait_for(ul, timeout, [] { return !inbox.empty(); }))
  {
    throw ConnectionFailed();
  }

  auto reply = dynamic_cast<ReplyMessage *>(inbox[0]); // May get an unexpected type?
  inbox.pop_back();

  if (reply->ref_message_id != message_id)
  {
    throw std::runtime_error("Wrong ref message id");
  }
  else if (reply->result != 1)
  {
    state = STATE_START;
    return 1;
  }

  message_id++;

  return 0; // OK
}
