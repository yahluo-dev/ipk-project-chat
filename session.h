#include <string>
#include <cstdint>
#include <netinet/in.h>
#include "server.h"

class Session
{
  private:
    int client_socket;
    uint16_t message_id;
    std::string username, secret, displayname;
    UDPServer server;
  public:
    Session(int _client_socket, std::string _username, std::string _secret, std::string _displayname);
    virtual int sendmsg(std::string contents);
    virtual int join(std::string channel_id, std::string displayname);
    virtual int rename(std::string new_name);
    virtual int auth();
};
