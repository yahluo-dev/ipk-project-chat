#ifndef SENDER_H
#define SENDER_H
#include "message.h"
#include <vector>
#include "session.h"

class UDPServer;
class Session;

enum sender_state_t
{
  STATE_WAITING,
  STATE_IDLE
};

class Sender
{
protected:
  static volatile sender_state_t state;
  Session *session;
  int sock;
public:
  Sender() = default;
  virtual void send_msg(MessageWithId *msg);
};

class UDPSender : public Sender
{
private:
  unsigned int max_retr;
  std::vector<ConfirmMessage *> confirm_inbox;
  MessageWithId *last_sent;
  std::chrono::milliseconds timeout;
public:
  struct addrinfo *server_addrinfo;
  UDPSender(int _sock, struct addrinfo *server_addrinfo, unsigned int _max_retr,
            std::chrono::milliseconds _timeout, Session *_session);
  void notify_confirm(ConfirmMessage *msg);
  void confirm(uint16_t ref_message_id);
  void send_msg(MessageWithId *msg) override;

  void update_addrinfo(const std::string &hostname, const std::string &port);
};

#endif // SENDER_H