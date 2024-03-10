#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <algorithm>
#include "exception.h"
#include "main.h"
#include "client.h"

#define DEFAULT_PORT "4567"

size_t message_id = 0;

int main(int argc, char *argv[])
{
  std::string port_number = DEFAULT_PORT;
  std::string server_hostname = "";
  std::string proto_str;

  Protocol proto = NONE;
  int udp_timeout = 250;
  int udp_max_retr = 3;
  int c, s;

  struct addrinfo hints;
  struct addrinfo *result;
  
  while ((c = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
  {
    switch(c)
    {
      case 't':
        proto_str = optarg;
        std::transform(proto_str.begin(), proto_str.end(), proto_str.begin(), ::toupper);
        if (!proto_str.compare("TCP")) proto = TCP;
        else if (!proto_str.compare("UDP")) proto = UDP;
        else
        {
          std::cerr << "-t: Please supply either \"-t udp\" or \"-t tcp\"" << std::endl;
          exit(1);
        }
        break;
      case 's':
        server_hostname = optarg;
        break;
      case 'p':
        if (atoi(optarg) > 65535 || atoi(optarg) < 1)
        {
          std::cerr << "-s: PORT must be in range <1, 65535>!" << std::endl;
          exit(1);
        }
        port_number = optarg;
        break;
      case 'd':
        udp_timeout = strtol(optarg, NULL, 10);
        break;
      case 'r':
        udp_max_retr = strtol(optarg, NULL, 10);
        break;
      case 'h':
        std::cout << USAGE;
        return 0;
        break;
    }
  }

  if (server_hostname.size() == 0)
  {
    throw std::runtime_error("HOSTNAME must be supplied!");
  }

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

  int rv;

  if (NULL != (rv = getaddrinfo(server_hostname.c_str(), port_number.c_str(), &hints, &result)))
  {
    std::cerr << "getaddrinfo: " << gai_strerror(rv);
    return 1;
  }

  UDPClient *client;

  if (proto == UDP) client = new UDPClient(*result);
  else if (proto == TCP)
  {
    throw new NotImplemented();
  }
  else
  {
    throw std::runtime_error("PROTOCOL must be supplied!");
  }

  client->repl();

  return 0;
}
