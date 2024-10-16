#include "tcp_receiver.h"
#include "cstring"
#include "exception.h"
#include "tcp_message_factory.h"
#include "session.h"

#define RECVMESSAGE_MAXLEN 2048

std::string TCPReceiver::received_data;

void TCPReceiver::receive(Session &session, int sock)
{
  ssize_t got_bytes;
  std::vector<std::string> messages;

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
      if (got_bytes == 0) break;

      received_data.append(buffer, got_bytes);
      size_t pos;
      while ((pos = received_data.find("\r\n")) != std::string::npos)
      {
        std::string message = received_data.substr(0, pos+2);
        received_data.erase(0, pos+2);
        messages.push_back(message);
      }
    }
    catch (...)
    {
      session.set_receiver_ex();
      return;
    }

    for (std::string &raw_message : messages)
    {
      TCPMessageFactory factory = TCPMessageFactory();
      std::unique_ptr<Message> parsed_message = factory.create(raw_message);
      session.notify_incoming(std::move(parsed_message));
    }
  }
}
