#ifndef UDP_SENDER_H
#define UDP_SENDER_H

#include <vector>
#include <chrono>

#include "sender.h"

class Session;

class UDPSender : public Sender
{
private:
  unsigned int max_retr;
  std::vector<std::unique_ptr<ConfirmMessage>> confirm_inbox;
  std::unique_ptr<MessageWithId> last_sent;
  std::chrono::milliseconds timeout;
  struct addrinfo *server_addrinfo;
public:
  UDPSender(int _sock, struct addrinfo *server_addrinfo, unsigned int _max_retr,
            std::chrono::milliseconds _timeout, Session &_session);
  void notify_confirm(ConfirmMessage &msg);
  void confirm(uint16_t ref_message_id);
  void send_msg(std::unique_ptr<MessageWithId> msg) override;

  void update_addrinfo(const std::string &hostname, const std::string &port);
  ~UDPSender(){};
};

#endif // UDP_SENDER_H
