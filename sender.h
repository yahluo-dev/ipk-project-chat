#ifndef SENDER_H
#define SENDER_H

#include "message.h"
#include <memory>

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
  Session &session;
  int sock;
public:
  Sender(Session &_session, int _sock)
    : session(_session), sock(_sock){};
  virtual void send_msg(std::unique_ptr<MessageWithId> msg);
  virtual ~Sender();
};

#endif // SENDER_H
