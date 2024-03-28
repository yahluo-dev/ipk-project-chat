#include "client.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <iterator>
#include <chrono>
#include "exception.h"

std::regex username_regex("[a-zA-Z0-9-]{1,20}", std::regex_constants::ECMAScript);
std::regex secret_regex("[a-zA-Z0-9-]{1,128}", std::regex_constants::ECMAScript);
std::regex displayname_regex(R"([!-~]{1,20})", std::regex_constants::ECMAScript);
std::regex message_content_regex(R"([ -~]{1,1400})", std::regex_constants::ECMAScript);

const char *help =  "Usage:\n"
                    "\t/auth USERNAME SECRET DISPLAYNAME - Authenticate using supplied credentials.\n"
                    "\t/join CHANNEL_ID - Join channel.\n"
                    "\t/rename DISPLAYNAME - Change current display name.\n"
                    "\t/help - Show this message.\n";


UDPClient::UDPClient(std::string hostname, std::string port, unsigned int _timeout, unsigned int _udp_max_retr)
{
  std::chrono::milliseconds timeout_ms(_timeout);

  session = new Session(hostname, port, _udp_max_retr,
                        timeout_ms);
}

void UDPClient::repl()
{
  std::string input;
  while(true)
  {
    std::cout << "IPK24-CHAT> ";
    std::getline(std::cin, input);
    if (std::cin.eof())
    {
      // TODO: Send bye to server
      session->bye();
      std::cout << "Bye!" << std::endl;
      std::cout << std::endl;
      return;
    }

    if (STATE_INTERNAL_ERROR == session->get_state())
    {
      throw ConnectionFailed();
    }

    if (input[0] == '/')
    {
      // Is a command. Break string on spaces.
      std::stringstream ss(input);
      ss.ignore(1); // Skip '/'
      std::istream_iterator<std::string> begin(ss);
      std::istream_iterator<std::string> end;
      std::vector<std::string> command_args(begin, end);

      // Auth
      if (command_args[0] == "auth")
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

        std::cout << "Authenticating..." << std::endl;

        if (0 != session->auth(username, secret, displayname))
        {
          std::cerr << "Authentication failed." << std::endl;
          continue;
        }
        std::cerr << "Authentication success." << std::endl;
      }
      else if (command_args[0] == "join")
      {
        if (command_args.size() != 2)
        {
          std::cerr << "Invalid usage." << std::endl;
          continue;
        }
        std::string channel_id = command_args[1];
        try
        {
          if (0 != session->join(channel_id))
          {
            std::cerr << "Join failed!" << std::endl;
          }
          else
          {
            std::cout << "Joined " << channel_id << std::endl;
          }
        }
        catch (NotAuthenticated &e)
        {
          std::cout << "Not authenticated. Use /auth." << std::endl;
        }
      }
      else if (command_args[0] == "rename")
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
      else if (command_args[0] == "help")
      {
        puts(help);
      }
      else
      {
        std::cerr << "Invalid command: " << command_args[0] <<"." << std::endl;
        puts(help);
      }
    }
    else if (input.empty())
    {
      continue;
    }
    else
    {
      // Is message. Send.
      if (!std::regex_match(input, message_content_regex))
      {
        std::cerr << "Invalid message content. Only characters from ASCII range \\x20-\\x7e are allowed." << std::endl;
        continue;
      }
      try
      {
        session->sendmsg(input);
      }
      catch (NotInChannel &e)
      {
        std::cout << "Cannot send message while not in channel." << std::endl;
      }
    }
  }
}
