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
class Client
{
protected:
  Session *session;
public:
  void repl();
  explicit Client(Session *_session) : session(_session) {};
};

#endif // CLIENT_H
