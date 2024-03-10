#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "session.h"

#define COMMAND_MAXLEN 1023

/**
 * Class for communicating with the user via a REPL and interpreting commands.
 */
class UDPClient
{
  private:
    Session *session;
    int client_socket;
    struct addrinfo server_addrinfo;
  public:
    UDPClient(struct addrinfo _server_addrinfo);
    void repl();
};
