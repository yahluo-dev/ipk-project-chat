#include "server.h"
#include "message_factory.h"
#include <stdexcept>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include "receiver.h"
#include <condition_variable>
#include <thread>
#include "sender.h"


#define RECVMESSAGE_MAXLEN 2048

std::condition_variable got_message_cv;

UDPServer::UDPServer()
{
  sock = -1; // TODO: Remove this
  // throw new std::logic_error("Should not get here.");
  // We get here when creating a new instance of session...
}

UDPServer::UDPServer(int _sock, int _max_retr, std::chrono::milliseconds _timeout)
{
  sock = _sock;
  max_retr = _max_retr;

  receiving_thread = std::jthread(Receiver::receive, this, sock);
  timeout = _timeout;

  sender = new UDPSender(sock, max_retr, timeout, this);
}

void notify_send_success()
{
}

void UDPServer::notify_incoming(Message *msg)
{
  if (msg->code == CODE_CONFIRM)
  {
    confirms.push_back(msg);
    // notify of new confirm
  }
  else
  {
    inbox.push_back(msg);
  }
}

/**
 * Send message
 */
void UDPServer::send_msg(Message *msg)
{
  sender->send_msg(msg);
}

/**
 * Check the message queue if got a confirm with this number
 */
ConfirmMessage *UDPServer::check_got_confirm(uint16_t ref_message_id)
{
  for (auto iter = inbox.begin(); iter != inbox.end();)
  {
    Message *msg = *iter;
    if (msg->code != CODE_CONFIRM)
      continue;
    ConfirmMessage *confirmation = dynamic_cast<ConfirmMessage *>(msg);
    if (confirmation->ref_message_id == ref_message_id)
    {
      return confirmation;
    }
    else if (confirmation->ref_message_id > ref_message_id)
    {
      throw new std::runtime_error("Got a confirm for a message not yet sent!");
    }
    else
    {
      iter = inbox.erase(iter);
      continue; // Do not increment iterator, already erased
    }

    iter++;
  }
  return NULL;
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
    return 0;
    //throw new std::runtime_error("server: recvmsg failed");
  }

  std::string binary_message = std::string(buffer, got_bytes);

  MessageFactory factory = MessageFactory();
  Message *parsed_message = factory.create(binary_message);

  return parsed_message;
}
