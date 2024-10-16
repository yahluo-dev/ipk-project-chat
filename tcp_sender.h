#ifndef IPK_PROJECT_1_TCP_SENDER_H
#define IPK_PROJECT_1_TCP_SENDER_H

#include "sender.h"

class TCPSender : public Sender
{
public:
  TCPSender(int _sock, Session &_session);
  void send_msg(std::unique_ptr<MessageWithId> msg) override;
  ~TCPSender(){};
};

#endif //IPK_PROJECT_1_TCP_SENDER_H
