#ifndef CLIENT_H
#define CLIENT_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include "tcp_session.h"
#include "udp_session.h"

/**
 * Class for communicating with the user via a REPL and interpreting commands.
 */
class Client
{
protected:
  static Session *session;
  static bool interrupted;
  void print_prompt();
public:
  void repl();
  explicit Client(Session *_session);
  static void ctrlc_handler(int signal);
};

#endif // CLIENT_H
