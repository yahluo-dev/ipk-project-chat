#include "udp_receiver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "message_factory.h"
#include "exception.h"
#include <cerrno>
#include <cstring>
#include <memory>
#include <arpa/inet.h>
#include "session.h"

#define RECVMESSAGE_MAXLEN 2048

void UDPReceiver::receive(Session *session, int sock, UDPSender *sender)
{
  char buffer[RECVMESSAGE_MAXLEN] = {0};
  struct msghdr msg = {0}; // GCC doesn't like this, C-style
  struct iovec iov = {0};
  char control_buffer[CMSG_SPACE(sizeof(struct sockaddr_in))];
  ssize_t got_bytes;

  while(true)
  {
    struct sockaddr_in client_addr = {0};
    iov.iov_base = buffer;
    iov.iov_len = RECVMESSAGE_MAXLEN;
    msg.msg_name = &client_addr;
    msg.msg_namelen = sizeof(client_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_buffer;
    msg.msg_controllen = sizeof(control_buffer);

    try
    {
      if (-1 == (got_bytes = recvmsg(sock, &msg, 0)))
      {
        printf("Receive message failed: %s\n", strerror(errno));
        fflush(stdout);
        throw ConnectionFailed();
      }
    }
    catch (ConnectionFailed &e)
    {
      session->set_receiver_ex();
      return;
    }
    // TODO: Keep track of retransmissions from server

    char client_addr_str[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, INET_ADDRSTRLEN);

    sender->update_addrinfo(std::string(client_addr_str), std::to_string(ntohs(client_addr.sin_port)));

    std::string binary_message = std::string(buffer, got_bytes);

    MessageFactory factory = MessageFactory();
    Message *parsed_message = factory.create(binary_message);

    if (parsed_message->code == CODE_CONFIRM)
    {
      sender->notify_confirm(dynamic_cast<ConfirmMessage *>(parsed_message));
    }
    else
    {
      sender->confirm(dynamic_cast<MessageWithId *>(parsed_message)->get_message_id());
      session->notify_incoming(parsed_message);
    }
    // Notify session
  }
}