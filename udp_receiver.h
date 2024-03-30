#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include "session.h"

class Sender; // TODO: Remove this, circ dep
class UDPSession;
class UDPSender;

class UDPReceiver
{
public:
  static void receive(UDPSession *session, int sock, UDPSender *sender);
};

#endif // UDP_RECEIVER_H