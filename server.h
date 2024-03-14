#include "message.h"
#include <sys/socket.h>

class UDPServer
{
  private:
    int sock;
  public:
    UDPServer();
    UDPServer(int _sock);
    void sendmsg(Message *msg);
    void send_expect_confirm(MessageWithId *msg);
    Message *get_msg();
};
