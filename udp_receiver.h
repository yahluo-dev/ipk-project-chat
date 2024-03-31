#ifndef UDP_RECEIVER_H
#define UDP_RECEIVER_H

#include "udp_sender.h"

class Session;

class UDPReceiver
{
public:
  static void receive(Session *session, int sock, UDPSender *sender);
};

#endif // UDP_RECEIVER_H