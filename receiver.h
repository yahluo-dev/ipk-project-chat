#ifndef RECEIVER_H
#define RECEIVER_H

#include <thread>
#include "server.h"
#include "session.h"

class Sender; // TODO: Remove this, circ dep
class UDPSession;

class UDPReceiver
{
public:
  static void receive(UDPSession *session, int sock, UDPSender *sender);
  UDPReceiver() = default;
  ~UDPReceiver() = default;
};

#endif // RECEIVER_H