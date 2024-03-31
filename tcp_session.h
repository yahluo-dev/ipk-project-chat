#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include "session.h"

class TCPSession : public Session
{
public:
  void wait_for_reply() override;
  TCPSession(const std::string &hostname, const std::string& port);

  void process_reply(ReplyMessage *reply) override;
};

#endif // TCP_SESSION_H