#include "server.h"
#include "message_factory.h"
#include <stdexcept>
#include <iostream>

UDPServer::UDPServer()
{
  sock = -1; // TODO: Remove this
}

UDPServer::UDPServer(int _sock)
{
  sock = _sock;
}

void UDPServer::sendmsg(Message *msg)
{
  std::string serialized = msg->serialize();
  send(sock, serialized.data(), serialized.size(), 0);
}

void UDPServer::send_expect_confirm(MessageWithId *msg)
{
  std::string serialized = msg->serialize();
  send(sock, serialized.data(), serialized.size(), 0);

  ConfirmMessage *confirmation = dynamic_cast<ConfirmMessage *>(recvmsg());

  if (confirmation->code != msg->code ||
      confirmation->ref_message_id != msg->message_id)
  {
    std::cerr << "DEBUG: Got something unexpected..." << std::endl;
    std::cout << "Got message with code " << std::to_string(confirmation->code) << std::endl;
    std::cout << "\t...and ref_message_id " << std::to_string(confirmation->ref_message_id) << std::endl;
  }
}

Message *UDPServer::recvmsg()
{
  char *buffer = new char[1024];

  size_t got_bytes = recv(sock, buffer, 1024, 0);

  if (got_bytes == 1024)
  {
    throw new std::logic_error("Got message that is too long!");
  }

  std::string binary_message = std::string(buffer, got_bytes);

  MessageFactory factory = MessageFactory();
  Message *parsed_message = factory.create(binary_message);

  return parsed_message;
}
