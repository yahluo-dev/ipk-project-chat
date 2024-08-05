#include "udp_session.h"
#include "exception.h"

#include <iostream>

UDPSession::UDPSession(const std::string &_hostname, const std::string& port, unsigned int _max_retr,
                       std::chrono::milliseconds _timeout) : Session(_hostname), timeout(_timeout)
{
  int rv;
  struct addrinfo hints = {0};
  server_addrinfo = nullptr;
  struct addrinfo *p;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  if (0 != (rv = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &server_addrinfo)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
  }
  for (p = server_addrinfo; p != nullptr; p = p->ai_next)
  {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
                                p->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }

    break;
  }
  sender = new UDPSender(client_socket, server_addrinfo, _max_retr, timeout, this);
  receiving_thread = std::jthread(UDPReceiver::receive, this, client_socket, dynamic_cast<UDPSender *>(sender));
}

void UDPSession::process_reply(ReplyMessage *reply)
{
  if (reply->get_ref_message_id() != message_id-1) // Does it match the previous message id?
  {
    // It does not. Probably an error on the server side.
    sender->send_msg(new ErrMessage(message_id++,
                                    display_name, "Reply contains wrong ref_message_id!"));
    std::cout << "ERR: Reply contains wrong ref_message_id!" << std::endl;
    state = STATE_ERROR;
    return;
  }
  else if (reply->get_result() == 0)
  {
    std::cerr << "Failure: " << reply->get_contents() << std::endl;
    state = STATE_START;
    return;
  }
  std::cerr << "Success: " << reply->get_contents() << std::endl;
  state = STATE_OPEN;
}

void UDPSession::wait_for_reply()
{
  std::unique_lock ul(inbox_mutex);

  if(!inbox_cv.wait_for(ul, timeout, [] { return !inbox.empty(); }))
  {
    throw ConnectionFailed();
  }
}
