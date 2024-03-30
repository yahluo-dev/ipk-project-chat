#include "tcp_receiver.h"
#include "cstring"
#include "exception.h"
#include "tcp_message_factory.h"

#define RECVMESSAGE_MAXLEN 2048

void TCPReceiver::receive(Session *session, int sock)
{
  ssize_t got_bytes;

  while(true)
  {
    char buffer[RECVMESSAGE_MAXLEN] = {0};
    try
    {
      if (-1 == (got_bytes = recv(
          sock, &buffer, RECVMESSAGE_MAXLEN, 0)))
      {
        printf("Receive message failed: %s\n", strerror(errno));
        fflush(stdout);
        throw ConnectionFailed();
      }
    }
    catch (...)
    {
      session->set_receiver_ex();
      return;
    }

    std::string raw_message = std::string(buffer, got_bytes);
    TCPMessageFactory factory = TCPMessageFactory();
    Message *parsed_message = factory.create(raw_message);
    session->notify_incoming(parsed_message);
  }
}