#include "tcp_session.h"
#include "tcp_sender.h"
#include "tcp_receiver.h"
#include "exception.h"

#include <iostream>
#include <mutex>


TCPSession::TCPSession(const std::string &hostname, const std::string& port)
  : Session(hostname)
{
  struct addrinfo hints = {0};
  int rv;
  struct addrinfo *p;
  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
    fflush(stderr);
    throw ConnectionFailed();
  }

  for(p = server_addrinfo; p != NULL; p = p->ai_next) {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1) {
      perror("client: socket");
      fflush(stderr);
      throw ConnectionFailed();
      continue;
    }

    if (connect(client_socket, p->ai_addr, p->ai_addrlen) == -1) {
      close(client_socket);
      perror("client: connect");
      fflush(stderr);
      throw ConnectionFailed();
      continue;
    }

    break;
  }

  sender = std::make_unique<TCPSender>(client_socket, *this);
  receiving_thread = std::jthread([this]() {
    TCPReceiver::receive(static_cast<Session&>(*this), this->client_socket);
  });
}

void TCPSession::wait_for_reply()
{
  std::unique_lock ul(inbox_mutex);

  inbox_cv.wait(ul, [] { return !inbox.empty(); });
  // Will wait indefinetely for a reply
}

void TCPSession::process_reply(ReplyMessage &reply)
{
  if (reply.get_result() == 0)
  {
    std::cerr << "Failure: " << reply.get_contents() << std::endl;
    state = STATE_START;
    return;
  }
  else
  {
    std::cerr << "Success: " << reply.get_contents() << std::endl;
    state = STATE_OPEN;
  }
}

TCPSession::~TCPSession()
{
  receiving_thread.join();
  close(client_socket);
}
