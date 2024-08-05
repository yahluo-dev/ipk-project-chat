#include "session.h"
#include "exception.h"
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <csignal>
#include "udp_sender.h"
#include "tcp_sender.h"

std::condition_variable Session::inbox_cv;
std::mutex Session::inbox_mutex;
session_state_t Session::state;
std::vector<Message *> Session::inbox;

void Session::set_receiver_ex()
{
  // Called by receiver, notify of error
  receiver_ex = std::current_exception();
  state = STATE_INTERNAL_ERROR;
  inbox_cv.notify_all(); // If the thread is waiting, free it
}

void Session::notify_incoming(Message *message)
{
  std::lock_guard<std::mutex> lg(inbox_mutex);
  if (message->get_code() == CODE_MSG)
  {
    auto msg_message = dynamic_cast<MsgMessage *>(message);
    std::cout << std::endl << msg_message->get_display_name() << ": " << msg_message->get_contents() << std::endl;
  }
  else if (message->get_code() == CODE_ERR)
  {
    state = STATE_ERROR;
    auto *err_message = dynamic_cast<ErrMessage *>(message);
    std::cerr << "ERR FROM " << err_message->get_display_name() <<
        ": " << err_message->get_contents() << std::endl;
    sender->send_msg(new ByeMessage(message_id++));
  }
  else if (message->get_code() == CODE_UNKNOWN)
  {
    state = STATE_ERROR;
    std::cerr << "ERR: Couldn't decode received message!" << std::endl;
  }
  inbox.push_back(message);
  inbox_cv.notify_one();
}


Session::~Session()
{
  receiving_thread.join();
  close(client_socket);
}

void Session::sendmsg(const std::string &_contents)
{
  if (state == STATE_START)
  {
    std::cerr << "ERR: Not in channel." << std::endl;
    return;
  }

  auto message = new MsgMessage(message_id++, display_name, _contents);
  try
  {
    sender->send_msg(message);
  }
  catch (BadConfirm &e)
  {
    state = STATE_ERROR;
  }
}

void Session::bye()
{
  if (state == STATE_START)
  {
    return;
  }
  auto *bye_message = new ByeMessage(message_id++);
  sender->send_msg(bye_message);
}

void Session::join(const std::string &_channel_id)
{
  if (state == STATE_START)
  {
    std::cerr << "ERR: Not authenticated. Use /auth." << std::endl;
    return;
  }

  MessageWithId *message = new JoinMessage(message_id++, _channel_id, display_name);
  sender->send_msg(message);

}

void Session::rename(const std::string &_new_name)
{
  display_name = _new_name;
}

session_state_t Session::get_state()
{
  return state;
}

void Session::auth(const std::string &_username, const std::string &_secret,
                  const std::string &_display_name)
{

  if (state != STATE_START)
  {
    std::cout << "ERR: Already authenticated." << std::endl;
    return;
  }

  username = _username;
  display_name = _display_name;
  secret = _secret;

  try
  {
    sender->send_msg(new AuthMessage(username, secret, display_name, message_id++));
  }
  catch (BadConfirm &e)
  {
    state = STATE_ERROR;
    sender->send_msg(new ErrMessage(message_id++,
                                    display_name, "Got bad confirm number."));
    return;
  }
  state = STATE_AUTH;

  wait_for_reply();

  if (state == STATE_ERROR)
  {
    sender->send_msg(new ErrMessage(message_id++,
                                    display_name, "REPLY was expected."));
    sender->send_msg(new ByeMessage(message_id++));
    return;
  }

  auto reply = dynamic_cast<ReplyMessage *>(inbox[0]);
  inbox.pop_back();

  process_reply(reply);
}
