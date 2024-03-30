#include "udp_sender.h"
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

std::mutex addrinfo_mutex;

volatile sender_state_t Sender::state;

void Sender::send_msg(MessageWithId *msg)
{
  throw NotImplemented();
}

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

  // Are we expecting a confirmation?
  if (state == STATE_WAITING) {
    if (msg->ref_message_id < last_sent->get_message_id()) {
      // Must be a duplicate or out of order
      return;
    } else if (msg->ref_message_id == last_sent->get_message_id()) {
      state = STATE_IDLE;
      confirm_cv.notify_one();
    } else if (msg->ref_message_id > last_sent->get_message_id()) {
      state = STATE_SENDER_ERROR;
    }
  }
}

void UDPSender::update_addrinfo(const std::string &hostname, const std::string &port)
{
  std::lock_guard<std::mutex> addrinfo_lock(addrinfo_mutex);
  struct addrinfo hints = {0};
  server_addrinfo = nullptr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  int rv;
  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
  }

}

void UDPSender::confirm(uint16_t ref_message_id)
{
  std::lock_guard<std::mutex> addrinfo_lock(addrinfo_mutex);
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

  unsigned int retries;

  for (retries = max_retr; retries > 0; retries--)
  {
    {
      std::lock_guard<std::mutex> addrinfo_lock(addrinfo_mutex);
      if (-1 == sendto(sock, serialized.data(), serialized.size(), 0,
                       server_addrinfo->ai_addr, server_addrinfo->ai_addrlen))
      {
        printf("Send message failed: %s\n", strerror(errno));
        fflush(stdout);
        throw ConnectionFailed();
      }
    }

    // Wait for confirm
    state = STATE_WAITING;
    // This condition is true when state == STATE_IDLE
    // (receiver already notified us of confirmation)
    //delete last_sent;
    last_sent = msg;
    if (confirm_cv.wait_for(ul, timeout,
                                    [] { return UDPSender::state != STATE_WAITING; }))
      {
        break;
      }
    if (state == STATE_SENDER_ERROR)
    {
      throw BadConfirm();
    }
  }

  if (retries == 0)
  {
    throw ConnectionFailed();
  }
  // Message sent successfully
}