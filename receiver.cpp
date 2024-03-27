#include "receiver.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include "message_factory.h"
#include "exception.h"
#include <cerrno>
#include <cstring>
#include <memory>

#define RECVMESSAGE_MAXLEN 2048

UDPReceiver::UDPReceiver()
{
  client_socket = -1;
  session = nullptr;
}

void UDPReceiver::receive(Session *session, int sock, UDPSender *sender)
{
  char buffer[RECVMESSAGE_MAXLEN] = {0};
  struct msghdr msg = {0}; // GCC doesn't like this, C-style
  struct iovec iov = {0};
  char control_buffer[CMSG_SPACE(sizeof(struct sockaddr_in))];
  ssize_t got_bytes; 

  while(true)
  {
    iov.iov_base = buffer;
    iov.iov_len = RECVMESSAGE_MAXLEN;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_buffer;
    msg.msg_controllen = sizeof(control_buffer);

    if (-1 == (got_bytes = recvmsg(sock, &msg, 0)))
    {
      printf("Receive message failed: %s\n", strerror(errno));
      fflush(stdout);
      throw ConnectionFailed();
    }
    // TODO: Keep track of retransmissions from server

    std::string binary_message = std::string(buffer, got_bytes);

    MessageFactory factory = MessageFactory();
    Message *parsed_message = factory.create(binary_message);

    if (parsed_message->code == CODE_CONFIRM)
    {
      sender->notify_confirm(dynamic_cast<ConfirmMessage *>(parsed_message));
    }
    else
    {
      session->notify_incoming(parsed_message);

      sender->confirm(dynamic_cast<MessageWithId *>(parsed_message)->message_id);
    }
    // Notify session
  }
}

UDPReceiver::UDPReceiver(int sock, Session *_session)
{
  client_socket = sock;
  session = _session;
}
