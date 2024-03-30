#ifndef SENDER_H
#define SENDER_H

#include "message.h"
#include "session.h"

class Session;

enum sender_state_t
{
  STATE_WAITING,
  STATE_IDLE,
  STATE_SENDER_ERROR
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

#endif // SENDER_H
