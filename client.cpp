#include "client.h"
#include <regex>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iterator>

std::regex username_regex("[a-zA-Z0-9-]{1,20}", std::regex_constants::ECMAScript);
std::regex secret_regex("[a-zA-Z0-9-]{1,128}", std::regex_constants::ECMAScript);
std::regex displayname_regex("[\x21-\x7e]{1,20}", std::regex_constants::ECMAScript);
std::regex message_content_regex("[\x20-\x7e]{1,1400}", std::regex_constants::ECMAScript);

const char *help =  "Usage:\n"
                    "\t/auth USERNAME SECRET DISPLAYNAME - Authenticate using supplied credentials.\n"
                    "\t/join CHANNEL_ID - Join channel.\n"
                    "\t/rename DISPLAYNAME - Change current display name.\n"
                    "\t/help - Show this message.\n";

void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET)
  {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

UDPClient::UDPClient(addrinfo _server_addrinfo)
{
  server_addrinfo = _server_addrinfo;
  char s[INET_ADDRSTRLEN];

  struct addrinfo *p;

  for (p = &_server_addrinfo; p != NULL; p = p->ai_next)
  {
    if ((client_socket = socket(p->ai_family, p->ai_socktype,
            p->ai_protocol)) == -1)
    {
      perror("client: socket");
      continue;
    }
    if (-1 == connect(client_socket, p->ai_addr, p->ai_addrlen))
    {
      perror("connect");
      continue;
    }
    break;
  }

  if (p == NULL)
  {
      perror("connect");
      exit(1);
  }

  std::cerr << "client: connecting to " 
    << inet_ntop(p->ai_family, get_in_addr(
          (struct sockaddr *)p->ai_addr), s, sizeof(s));

  session = NULL;
}

void printVector(const std::vector<std::string>& vec) {
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i != vec.size() - 1)
            std::cout << ", ";
    }
    std::cout << " ]" << std::endl;
}

void UDPClient::repl()
{
  std::string input = "";
  while(1)
  {
    // 
    std::cout << "IPK24-CHAT> ";
    std::getline(std::cin, input);
    if (std::cin.eof())
    {
      throw std::runtime_error("Input ended.");
    }

    if (input[0] == '/')
    {
      // Is a command. Break string on spaces.
      std::stringstream ss(input);
      ss.ignore(1); // Skip '/'
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> command_args(begin, end);
      printVector(command_args);

      // Auth
      if (!command_args[0].compare("auth"))
      {
        if (command_args.size() != 4)
        {
          std::cerr << "Invalid usage." << std::endl;
          continue;
        }
        std::string username = command_args[1];
        std::string secret = command_args[2];
        std::string displayname = command_args[3];

        if (!std::regex_match(username, username_regex))
        {
          std::cerr << "Username must match [a-zA-Z0-9-]+." << std::endl;
          continue;
        }
        else if (!std::regex_match(secret, secret_regex))
        {
          std::cerr << "Secret must match [a-zA-Z0-9-]+." << std::endl;
          continue;
        }
        else if (!std::regex_match(displayname, displayname_regex))
        {
          std::cerr << "Display name must match [\\x21-\\x7e]+." << std::endl;
          continue;
        }

        Session *new_session = new Session(client_socket, username, secret, displayname);

        if (0 != new_session->auth())
        {
          std::cerr << "Authentication failed." << std::endl;
          continue;
        }
        std::cerr << "Authentication success." << std::endl;

        session = new_session;
      }
      else if (!command_args[0].compare("join"))
      {
        if (command_args.size() != 3)
        {
          std::cerr << "Invalid usage." << std::endl;
          continue;
        }

        if (NULL == session)
        {
          std::cerr << "Not connected to any server!";
        }

        std::string channel_id = command_args[1];
        std::string displayname = command_args[2];

        if (0 != session->join(channel_id, displayname))
        {
          std::cerr << "Join failed!" << std::endl;
        }
      }
      else if (!command_args[0].compare("rename"))
      {
        if (command_args.size() != 2)
        {
          std::cerr << "Invalid usage." << std::endl;
          continue;
        }

        std::string displayname = command_args[1];

        if (0 != session->rename(displayname))
        {
          std::cerr << "Rename failed!" << std::endl;
        }
      }
      else if (!command_args[0].compare("help"))
      {
        puts(help);
      }
      else
      {
        std::cerr << "Invalid command: " << command_args[0] <<"." << std::endl;
        puts(help);
      }
    }
    else
    {
      // Is message. Send.
      if (!std::regex_match(input, message_content_regex))
      {
        std::cerr << "Invalid message content. Only characters from ASCII range \\x20-\\x7e are allowed." << std::endl;
        continue;
      }
      if (NULL == session)
      {
        std::cerr << "You are not authenticated!" << std::endl;
        continue;
      }
      session->sendmsg(input);
    }
  }
}
