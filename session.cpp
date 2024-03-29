#include "session.h"
#include "exception.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <arpa/inet.h>
#include "sender.h"
#include "receiver.h"
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
    std::cout << msg_message->display_name << ": " << msg_message->message_contents << std::endl;
  }
  inbox.push_back(message);
  std::cerr << "Notifying inbox" << std::endl;
  inbox_cv.notify_one();
}

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
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
  close(client_socket);
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

  auto message = new MsgMessage(message_id, display_name, _contents);
  std::cout << "DEBUG: Sending message with contents" << " " << _contents << std::endl;
  sender->send_msg(message);

  message_id++;

  return 1;
}

void Session::bye()
{
  auto *bye_message = new ByeMessage(message_id);
  sender->send_msg(bye_message);
}

int Session::join(const std::string &_channel_id)
{
  if (state == STATE_START)
  {
    throw NotAuthenticated();
  }

  MessageWithId *message = new JoinMessage(message_id, _channel_id, display_name);
  sender->send_msg(message);

  // TODO

  message_id++;

  return 0;
}

int Session::rename(const std::string &_new_name)
{
  display_name = _new_name;
  return 0;
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

int TCPSession::auth(const std::string &_username, const std::string &_secret,
                  const std::string &_display_name)
{

  if (state != STATE_START)
  {
    std::cout << "Already authenticated." << std::endl;
    return 0;
  }

  username = _username;
  display_name = _display_name;
  secret = _secret;

  sender->send_msg(new AuthMessage(username, secret, display_name, message_id));
  state = STATE_AUTH;

  wait_for_reply();

  auto reply = dynamic_cast<ReplyMessage *>(inbox[0]); // May get an unexpected type?
  inbox.pop_back();

  if (reply->result != 1)
  {
    state = STATE_START;
    return 1;
  }

  message_id++;
  state = STATE_OPEN;
  return 0; // OK
}
int UDPSession::auth(const std::string &_username, const std::string &_secret,
                  const std::string &_display_name)
{

  if (state != STATE_START)
  {
    std::cout << "Already authenticated." << std::endl;
    return 0;
  }

  username = _username;
  display_name = _display_name;
  secret = _secret;

  sender->send_msg(new AuthMessage(username, secret, display_name, message_id));
  state = STATE_AUTH;

  wait_for_reply();

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
  state = STATE_OPEN;
  return 0; // OK
}
