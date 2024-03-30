#ifndef UDP_SENDER_H
#define UDP_SENDER_H
#include <vector>
#include "sender.h"

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

#endif // UDP_SENDER_H