#include <iostream>
#include <unistd.h>
#include <algorithm>
#include "exception.h"
#include "main.h"
#include "client.h"
#include <climits>

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
  int opt_char;
  while ((opt_char = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
  {
    switch(opt_char)
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
      {
        long arg_port = strtol(optarg, nullptr, 10);
        if (arg_port > 65535 || arg_port < 1)
        {
          std::cerr << "-s: PORT must be in range <1, 65535>!" << std::endl;
          exit(1);
        }
        port_number = optarg;
        break;
      }
      case 'd':
      {
        long arg_val = strtol(optarg, nullptr, 10);
        if (arg_val < 0 || arg_val > UINT_MAX)
        {
          std::cerr << "-d: Timeout must be positive!" << std::endl;
          exit(1);
        }
        udp_timeout = arg_val;
        break;
      }
      case 'r':
      {
        long arg_val = strtol(optarg, nullptr, 10);
        if (arg_val < 1 || arg_val > UINT_MAX)
        {
          std::cerr << "-r: Max retries must be >=1!" << std::endl;
          exit(1);
        }
        udp_max_retr = arg_val;
        break;
      }
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
    std::cerr << "HOSTNAME must be supplied!" << std::endl;
    exit(1);
  }
  UDPClient *client;

  if (proto == UDP) client = new UDPClient(server_hostname, port_number, udp_timeout, udp_max_retr);
  else if (proto == TCP)
  {
    throw NotImplemented();
  }
  else
  {
    std::cerr << "PROTOCOL must be supplied!" << std::endl;
  }

  std::cout << "Use /help to get help. Exit with ^D." << std::endl;

  try
  {
    client->repl();
  }
  catch (ConnectionFailed &e)
  {
    std::cout << "Fatal connection error." << std::endl;
  }
}
