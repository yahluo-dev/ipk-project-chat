#include "server.h"

UDPServer::UDPServer(int _sock)
{
  sock = _sock;
}

void UDPServer::send_msg(Message *msg)
{
  // message_t *serialized = msg->serialize();
  // send(sock, serialized, msg->serialized_size(), 0);
}
