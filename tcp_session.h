#ifndef TCP_SESSION_H
#define TCP_SESSION_H

#include "session.h"

class TCPSession : public Session
{
private:
  struct addrinfo *server_addrinfo;
  void wait_for_reply() override;
  void process_reply(ReplyMessage &reply) override;
public:
  TCPSession(const std::string &hostname, const std::string& port);
  ~TCPSession();
};

#endif // TCP_SESSION_H
