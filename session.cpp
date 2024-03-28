#include "session.h"
#include "exception.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <arpa/inet.h>
#include "sender.h"
#include "receiver.h"

std::condition_variable inbox_cv;
std::mutex inbox_mutex;

volatile session_state_t Session::state;
std::vector<Message *> Session::inbox;

void Session::set_receiver_ex()
{
  // called by receiver
  receiver_ex = std::current_exception();
  state = STATE_INTERNAL_ERROR;
  inbox_cv.notify_one(); // If the thread is waiting, free it
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

void Session::update_port(const std::string &port)
{
  struct addrinfo hints = {0};
  server_addrinfo = nullptr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  int rv;
  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
  }

  sender->server_addrinfo = server_addrinfo;
}

Session::Session(const std::string &_hostname, const std::string& port, unsigned int _max_retr, std::chrono::milliseconds _timeout)
{
  int rv;
  struct addrinfo hints = {0};
  server_addrinfo = nullptr;
  struct addrinfo *p;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  char s[INET_ADDRSTRLEN];
  hostname = _hostname;
  max_retr = _max_retr;

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

  if (p == nullptr)
  {
    perror("connect");
    exit(1);
  }

  std::cerr << "client: connecting to "
            << inet_ntop(p->ai_family, get_in_addr(
                (struct sockaddr *)p->ai_addr), s, sizeof(s))
            << std::endl;

  std::cerr << "DEBUG: Created session. Server with " << _max_retr << " retr." << std::endl;
  timeout = _timeout;

  sender = new UDPSender(client_socket, server_addrinfo, _max_retr, timeout, this);
  receiver = new UDPReceiver(client_socket, this);

  receiving_thread = std::jthread(UDPReceiver::receive, this, client_socket, sender);

  message_id = 1;
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

int Session::auth(const std::string &_username, const std::string &_secret,
                  const std::string &_display_name)
{
  std::unique_lock ul(inbox_mutex);

  if (state != STATE_START)
  {
    std::cout << "Already authenticated." << std::endl;
  }

  username = _username;
  display_name = _display_name;
  secret = _secret;

  MessageWithId *message = new AuthMessage(username, secret, display_name, message_id);
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
