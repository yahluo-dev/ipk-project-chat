#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "session.h"

/**
 * Class for communicating with the user via a REPL and interpreting commands.
 */
class UDPClient
{
  private:
    Session *session;
    int client_socket;
  public:
    UDPClient(struct addrinfo _server_addrinfo, unsigned int timeout, unsigned int udp_max_retr);
    void repl();
};

#endif // CLIENT_H
