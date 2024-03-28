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
  public:
    UDPClient(std::string hostname, std::string port, unsigned int timeout, unsigned int udp_max_retr);
    void repl();
};

#endif // CLIENT_H
