#ifndef UDP_SESSION
#define UDP_SESSION

#include "session.h"
#include "udp_receiver.h"
#include "udp_sender.h"

class UDPSession : public Session
{
private:
  struct addrinfo *server_addrinfo;
  std::chrono::milliseconds timeout;
  void wait_for_reply() override;
  void process_reply(ReplyMessage *reply) override;
public:
  UDPSession(const std::string &hostname, const std::string& port,
             unsigned int _max_retr, std::chrono::milliseconds _timeout);
  ~UDPSession();
};

#endif // UDP_SESSION
