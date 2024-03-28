#include <iostream>
#include <sys/socket.h>
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
  std::string server_hostname;
  std::string proto_str;

  Protocol proto = NONE;
  unsigned int udp_timeout = 5000;
  unsigned int udp_max_retr = 3;
  int c;
  while ((c = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
  {
    switch(c)
    {
      case 't':
        proto_str = optarg;
        std::transform(proto_str.begin(), proto_str.end(), proto_str.begin(), ::toupper);
        if (proto_str == "TCP") proto = TCP;
        else if (proto_str == "UDP") proto = UDP;
        else
        {
          std::cerr << R"(-t: Please supply either "-t udp" or "-t tcp")" << std::endl;
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
        udp_timeout = strtoul(optarg, nullptr, 10);
        break;
      case 'r':
        udp_max_retr = strtoul(optarg, nullptr, 10);
        break;
      case 'h':
        std::cout << USAGE;
        return 0;
      default:
        std::cerr << "Invalid Usage." << std::endl;
        std::cout << USAGE;
        return 1;
    }
  }

  if (server_hostname.empty())
  {
    throw std::runtime_error("HOSTNAME must be supplied!");
  }
  UDPClient *client;

  if (proto == UDP) client = new UDPClient(server_hostname, port_number, udp_timeout, udp_max_retr);
  else if (proto == TCP)
  {
    throw NotImplemented();
  }
  else
  {
    throw std::runtime_error("PROTOCOL must be supplied!");
  }

  try
  {
    client->repl();
  }
  catch (ConnectionFailed &e)
  {
    std::cout << "Fatal connection error." << std::endl;
  }
}
