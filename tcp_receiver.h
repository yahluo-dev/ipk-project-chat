#ifndef TCP_RECEIVER_H
#define TCP_RECEIVER_H

class Session;

class TCPReceiver
{
private:
public:
  static void receive(Session *session, int sock);
};

#endif // TCP_RECEIVER_H
