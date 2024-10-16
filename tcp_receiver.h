#ifndef TCP_RECEIVER_H
#define TCP_RECEIVER_H

#include <string>
#include <memory>

class Session;

class TCPReceiver
{
private:
  static std::string received_data;
public:
  static void receive(Session& session, int sock);
};

#endif // TCP_RECEIVER_H
