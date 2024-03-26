#ifndef RECEIVER_H
#define RECEIVER_H

#include <thread>
#include "server.h"

class UDPServer; // TODO: Remove this, circ dep
class Session;

class UDPReceiver
{
  private:
    int client_socket;
    std::thread receiving_thread;
    Session *session;
  public:
    static void receive(Session *session, int sock, UDPSender *sender);
    UDPReceiver();
    UDPReceiver(int sock, Session *session);
    ~UDPReceiver();
};

#endif // RECEIVER_H