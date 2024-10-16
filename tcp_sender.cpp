#include "tcp_sender.h"
#include <sys/socket.h>

TCPSender::TCPSender(int _sock, Session &_session) : Sender(_session, _sock)
{}

void TCPSender::send_msg(std::unique_ptr<MessageWithId> msg)
{
  std::string tcp_message = msg->make_tcp();

  send(sock, tcp_message.data(), tcp_message.size(), 0);
}
