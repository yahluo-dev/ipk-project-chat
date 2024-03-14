#include "server.h"
#include "message_factory.h"
#include <stdexcept>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>

#define RECVMESSAGE_MAXLEN 2048

UDPServer::UDPServer()
{
  sock = -1; // TODO: Remove this
}

UDPServer::UDPServer(int _sock)
{
  sock = _sock;
}

/**
 * Send message
 */
void UDPServer::sendmsg(Message *msg)
{
  std::string serialized = msg->serialize();
  send(sock, serialized.data(), serialized.size(), 0);
}

/**
 * Send a message and expect a CONFIRM with the corresponding ref_message_id
 */
void UDPServer::send_expect_confirm(MessageWithId *msg)
{
  std::string serialized = msg->serialize();
  send(sock, serialized.data(), serialized.size(), 0);

  ConfirmMessage *confirmation = dynamic_cast<ConfirmMessage *>(get_msg());

  std::cout << "Message id: " << std::to_string(msg->message_id) << std::endl;

  if (confirmation->code != CODE_CONFIRM ||
      confirmation->ref_message_id != msg->message_id)
  {
    std::cerr << "DEBUG: Got something unexpected..." << std::endl;
    std::cout << "Got message with code " << std::to_string(confirmation->code) << std::endl;
    std::cout << "\t...and ref_message_id " << std::to_string(confirmation->ref_message_id) << std::endl;
  }
}

/**
 * Receive one datagram from socket sock
 */
Message *UDPServer::get_msg()
{
  char buffer[RECVMESSAGE_MAXLEN] = {0};
  struct msghdr msg = {0};
  struct iovec iov;
  char control_buffer[CMSG_SPACE(sizeof(struct sockaddr_in))];
  ssize_t got_bytes; 

  iov.iov_base = buffer;
  iov.iov_len = RECVMESSAGE_MAXLEN;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control_buffer;
  msg.msg_controllen = sizeof(control_buffer);


  if (-1 == (got_bytes = recvmsg(sock, &msg, 0)))
  {
    throw new std::runtime_error("server: recvmsg failed");
  }

  std::string binary_message = std::string(buffer, got_bytes);

  MessageFactory factory = MessageFactory();
  Message *parsed_message = factory.create(binary_message);

  return parsed_message;
}
