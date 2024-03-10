#include "message.h"
#include <sys/socket.h>

class UDPServer
{
  private:
    int sock;
  public:
    UDPServer(int _sock);
    void send_msg(Message *msg);
};
