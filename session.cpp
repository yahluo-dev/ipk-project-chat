#include "session.h"
#include "exception.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <arpa/inet.h>
#include "udp_sender.h"
#include "udp_receiver.h"
#include "tcp_receiver.h"
#include "tcp_sender.h"

std::condition_variable inbox_cv;
std::mutex inbox_mutex;
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
  if (message->code == CODE_MSG)
  {
    auto msg_message = dynamic_cast<MsgMessage *>(message);
    std::cout << std::endl << msg_message->display_name << ": " << msg_message->message_contents << std::endl;
  }
  else if (message->code == CODE_ERR)
  {
    state = STATE_ERROR;
    auto *err_message = dynamic_cast<ErrMessage *>(message);
    std::cerr << "ERR FROM " << err_message->display_name <<
        ": " << err_message->message_contents << std::endl;
    sender->send_msg(new ByeMessage(message_id++));
  }
  else if (message->code == CODE_UNKNOWN)
  {
    state = STATE_ERROR;
    sender->send_msg(new ErrMessage(message_id++,
                                    display_name, "Got message with invalid code."));
    sender->send_msg(new ByeMessage(message_id++));
    std::cerr << "ERR: Got message with invalid code from server." << std::endl;
  }
  inbox.push_back(message);
  inbox_cv.notify_one();
}

UDPSession::UDPSession(const std::string &_hostname, const std::string& port, unsigned int _max_retr,
                       std::chrono::milliseconds _timeout) : Session(_hostname),
                       max_retr(_max_retr), timeout(_timeout)
{
  int rv;
  struct addrinfo hints = {0};
  server_addrinfo = nullptr;
  struct addrinfo *p;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
  }
  for (p = server_addrinfo; p != nullptr; p = p->ai_next)
  {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }

    break;
  }
  sender = new UDPSender(client_socket, server_addrinfo, _max_retr, timeout, this);
  receiving_thread = std::jthread(UDPReceiver::receive, this, client_socket, dynamic_cast<UDPSender *>(sender));
}

TCPSession::TCPSession(const std::string &hostname, const std::string& port) : Session(hostname)
{
  struct addrinfo hints = {0};
  int rv;
  struct addrinfo *p;
  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
  }

  for(p = server_addrinfo; p != NULL; p = p->ai_next) {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(client_socket, p->ai_addr, p->ai_addrlen) == -1) {
      close(client_socket);
      perror("client: connect");
      continue;
    }

    break;
  }

  sender = new TCPSender(client_socket, this);
  receiving_thread = std::jthread(TCPReceiver::receive, this, client_socket);
}

Session::~Session()
{
  //receiving_thread.join(); // FIXME: This is bad, closing a socket
  close(client_socket);      // while the thread is receiving.
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

void UDPSession::wait_for_reply()
{
  std::unique_lock ul(inbox_mutex);

  if(!inbox_cv.wait_for(ul, timeout, [] { return !inbox.empty(); }))
  {
    throw ConnectionFailed();
  }
}

void TCPSession::wait_for_reply()
{
  std::unique_lock ul(inbox_mutex);

  inbox_cv.wait(ul, [] { return !inbox.empty(); });
  // Will wait indefinetely for a reply
}

void TCPSession::process_reply(ReplyMessage *reply)
{
  if (reply->result == 0)
  {
    std::cout << "Failure: " << reply->message_contents << std::endl;
    state = STATE_START;
    return;
  }
  else
  {
    std::cout << "Success: " << reply->message_contents << std::endl;
    state = STATE_OPEN;
  }
}

void UDPSession::process_reply(ReplyMessage *reply)
{
  if (reply->ref_message_id != message_id-1) // Does it match the PREVIOUS message id?
  {
    sender->send_msg(new ErrMessage(message_id++,
                                    display_name, "Reply contains wrong ref_message_id!"));
    std::cout << "ERR: Reply contains wrong ref_message_id!" << std::endl;
    state = STATE_ERROR;
    // bye from cient?
    return;
  }
  else if (reply->result == 0)
  {
    std::cerr << "Failure: " << reply->message_contents << std::endl;
    state = STATE_START;
    return;
  }
  std::cerr << "Success: " << reply->message_contents << std::endl;
  state = STATE_OPEN;
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
    throw UnexpectedMessage();
    return;
  }

  auto reply = dynamic_cast<ReplyMessage *>(inbox[0]); // May get an unexpected type?
  inbox.pop_back();

  process_reply(reply);
}
