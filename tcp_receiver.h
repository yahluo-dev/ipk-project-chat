#ifndef IPK_PROJECT_1_TCP_RECEIVER_H
#define IPK_PROJECT_1_TCP_RECEIVER_H

#include "session.h"

class TCPReceiver
{
private:
public:
  static void receive(Session *session, int sock);
};

#endif //IPK_PROJECT_1_TCP_RECEIVER_H
