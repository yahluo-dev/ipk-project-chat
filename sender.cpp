#include "sender.h"
#include <sys/socket.h>
#include <condition_variable>
#include <mutex>
#include <stdexcept>
#include "exception.h"
#include "session.h"
#include <memory>
#include <cstring> // strerrno
#include <iostream>

std::condition_variable confirm_cv;
std::mutex confirm_mutex;

volatile sender_state_t UDPSender::state;

UDPSender::UDPSender(int _sock, struct addrinfo *_server_addrinfo, unsigned int _max_retr,
                     std::chrono::milliseconds _timeout, Session *_session)
{
  sock = _sock;
  max_retr = _max_retr;
  state = STATE_IDLE;
  timeout = _timeout;
  session = _session;
  last_sent = new MessageWithId((message_code_t)0x77, 0);
  server_addrinfo = _server_addrinfo;
}

void UDPSender::notify_confirm(ConfirmMessage *msg)
{
  std::lock_guard<std::mutex> lg(confirm_mutex);

  // Are we expecting a confirm?
  if (state == STATE_WAITING) {
    if (msg->ref_message_id < last_sent->message_id) {
      // Must be a duplicate or out of order
      return;
    } else if (msg->ref_message_id == last_sent->message_id) {
      state = STATE_IDLE;
      confirm_cv.notify_one();
    } else if (msg->ref_message_id > last_sent->message_id) {
      throw std::runtime_error("Got confirm for message not yet sent!");
    }
  }
}

void UDPSender::confirm(uint16_t ref_message_id)
{
  std::unique_ptr<ConfirmMessage> confirmation = std::make_unique<ConfirmMessage>(ref_message_id);
  std::string serialized = confirmation->serialize();

  if (-1 == sendto(sock, serialized.data(), serialized.size(), 0,
                   server_addrinfo->ai_addr, server_addrinfo->ai_addrlen))
  {
    printf("Send message failed: %s\n", strerror(errno));
    fflush(stdout);
    throw ConnectionFailed();
  }
}

void UDPSender::send_msg(MessageWithId *msg)
{
  std::string serialized = msg->serialize();
  std::unique_lock ul(confirm_mutex);

  int retries;

  for (; retries > 0;)
  {
    std::cout << "Sending" << std::endl;

    if (-1 == sendto(sock, serialized.data(), serialized.size(), 0,
                     server_addrinfo->ai_addr, server_addrinfo->ai_addrlen))
    {
      printf("Send message failed: %s\n", strerror(errno));
      fflush(stdout);
      throw ConnectionFailed();
    }

    // Wait for confirm
    state = STATE_WAITING;
    // This condition is true when state == STATE_IDLE
    // (receiver already notified us of confirmation)
    //delete last_sent;
    last_sent = msg;
    if (confirm_cv.wait_for(ul, timeout,
                                    [] { return UDPSender::state == STATE_IDLE; }))
      {
        break;
      }
    retries--;
    if (STATE_INTERNAL_ERROR == session->get_state())
    {
      return;
    }
  }
  if (retries == 0)
  {
    throw ConnectionFailed();
  }

  // Message sent successfully
}