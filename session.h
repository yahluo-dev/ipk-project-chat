#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <cstdint>
#include <netinet/in.h>
#include <vector>
#include <thread>
#include <exception>
#include <netdb.h>
#include <condition_variable>
#include <mutex>

#define debug_logf(format, ...) printf("DEBUG() :: " format "\n", __VA_ARGS__)
#define debug_log(message) printf("DEBUG() :: " message "\n")

#include "message.h"
#include "sender.h"

enum session_state_t
{
  STATE_START,
  STATE_AUTH,
  STATE_OPEN,
  STATE_ERROR,
  STATE_INTERNAL_ERROR,
  STATE_END
};

class Session
{
protected:
  static std::condition_variable inbox_cv;
  static std::mutex inbox_mutex;
  std::string username, secret, display_name, hostname;
  Sender *sender;
  int client_socket;
  uint16_t message_id;
  static session_state_t state;
  static std::vector<Message *> inbox;
  std::exception_ptr receiver_ex;

  virtual void wait_for_reply() = 0;
  virtual void process_reply(ReplyMessage *reply) = 0;
public:
  std::jthread receiving_thread;

  Session(const std::string &_hostname) : hostname(_hostname),
      sender(nullptr), client_socket(0), message_id(0){};
  virtual ~Session();
  virtual session_state_t get_state();

  virtual void sendmsg(const std::string &contents);
  virtual void join(const std::string &channel_id);
  virtual void rename(const std::string &new_name);
  virtual void auth(const std::string &_username, const std::string &_secret,
                    const std::string &_displayname);
  virtual void bye();

  virtual void set_receiver_ex();
  void notify_incoming(Message *message);
};

#endif // SESSION_H
