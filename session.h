#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <cstdint>
#include <netinet/in.h>
#include "server.h"
#include "message.h"
#include <vector>
#include <thread>
#include "receiver.h"
#include "sender.h"
#include <exception>
#include <netdb.h>

class UDPReceiver;
class Sender;

enum session_state_t
{
  STATE_START,
  STATE_AUTH,
  STATE_OPEN,
  STATE_ERROR,
  STATE_INTERNAL_ERROR
};

class Session
{
protected:
  std::string username, secret, display_name, hostname;
  struct addrinfo *server_addrinfo;
  UDPReceiver *receiver;
  Sender *sender;
  int client_socket;
  uint16_t message_id;
  static session_state_t state;
  std::jthread receiving_thread;
  static std::vector<Message *> inbox;
  std::exception_ptr receiver_ex;
public:
  Session(const std::string &_hostname) : hostname(_hostname),
      server_addrinfo(nullptr), receiver(nullptr), sender(nullptr),
      client_socket(0), message_id(1){};
  ~Session();
  virtual int sendmsg(const std::string &contents);
  virtual int join(const std::string &channel_id);
  virtual int rename(const std::string &new_name);
  virtual int auth(const std::string &_username, const std::string &_secret, const std::string &_displayname) = 0;
  virtual void set_receiver_ex();
  virtual void bye();

  virtual session_state_t get_state();
  void notify_incoming(Message *message);
  virtual void wait_for_reply() = 0;
};

class UDPSession : public Session
{
private:
  unsigned int max_retr;
  std::chrono::milliseconds timeout;
  void wait_for_reply();
public:
  UDPSession(const std::string &hostname, const std::string& port,
             unsigned int _max_retr, std::chrono::milliseconds _timeout);
  void update_port(const std::string &port);

  int auth(const std::string &_username, const std::string &_secret, const std::string &_display_name);
};

class TCPSession : public Session
{
public:
  void wait_for_reply();
  TCPSession(const std::string &hostname, const std::string& port);

  int auth(const std::string &_username, const std::string &_secret, const std::string &_display_name);
};
#endif // SESSION_H
