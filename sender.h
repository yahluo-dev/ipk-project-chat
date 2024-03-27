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

class UDPSender
{
private:
  int sock;
  unsigned int max_retr;
  std::vector<ConfirmMessage *> confirm_inbox;
  static volatile sender_state_t state;
  MessageWithId *last_sent;
  Session *session;
  std::chrono::milliseconds timeout;
public:
  UDPSender(int _sock, unsigned int _max_retr,
            std::chrono::milliseconds _timeout, Session *_session);
  void notify_confirm(ConfirmMessage *msg);
  void send_msg(MessageWithId *msg);
  void confirm(uint16_t ref_message_id);
};

#endif // SENDER_H