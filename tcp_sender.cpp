#include "tcp_sender.h"

TCPSender::TCPSender(int _sock, Session *_session)
{
  sock = _sock;
  session = _session;
}

void TCPSender::send_msg(MessageWithId *msg)
{
  std::string tcp_message = msg->make_tcp();

  send(sock, tcp_message.data(), tcp_message.size(), 0);
}