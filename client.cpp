#include "client.h"
#include <regex>
#include <iostream>
#include <sstream>
#include <iterator>
#include "exception.h"
#include <csignal>

std::regex username_regex("[a-zA-Z0-9-]{1,20}", std::regex_constants::ECMAScript);
std::regex secret_regex("[a-zA-Z0-9-]{1,128}", std::regex_constants::ECMAScript);
std::regex display_name_regex(R"([!-~]{1,20})", std::regex_constants::ECMAScript);
std::regex message_content_regex(R"([ -~]{1,1400})", std::regex_constants::ECMAScript);

const char *help =  "Usage:\n"
                    "\t/auth USERNAME SECRET DISPLAYNAME - Authenticate using supplied credentials.\n"
                    "\t/join CHANNEL_ID - Join channel.\n"
                    "\t/rename DISPLAYNAME - Change current display name.\n"
                    "\t/help - Show this message.\n";

std::unique_ptr<Session> Client::session;
bool Client::interrupted = false;

void Client::ctrlc_handler(int signal)
{
  if (interrupted)
  {
    std::cout << "Force quitting." << std::endl;
    exit(1);
  }
  interrupted = true;
  std::cout << "Bye!" << std::endl;
  Client::session->bye();
  exit(0);
}

Client::Client(std::unique_ptr<Session> _session)
{
  session = std::move(_session);
}

void Client::print_prompt()
{
  std::cout << "IPK24-CHAT ";
  switch(session->get_state())
  {
    case STATE_ERROR:
      std::cout << "[ERROR!]";
      break;
    default:
      break;
  }
  std::cout << "> ";
};

void Client::repl()
{
  std::signal(SIGINT, Client::ctrlc_handler);
  std::cout << "Use /help to get help. Exit with ^D or ^C." << std::endl;

  std::string input;
  while(true)
  {
    if (interrupted) return;
    if (STATE_INTERNAL_ERROR == session->get_state() ||
        STATE_ERROR == session->get_state())
    {
      throw ConnectionFailed();
    }
    print_prompt();
    std::getline(std::cin, input);
    if (std::cin.eof())
    {
      session->bye();
      std::cout << "Bye!" << std::endl;
      std::cout << std::endl;
      return;
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
          std::cerr << "ERR: Invalid usage." << std::endl;
          continue;
        }
        std::string username = command_args[1];
        std::string secret = command_args[2];
        std::string display_name = command_args[3];

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
        else if (!std::regex_match(display_name, display_name_regex))
        {
          std::cerr << "Display name must match [\\x21-\\x7e]+." << std::endl;
          continue;
        }

        session->auth(username, secret, display_name);
      }
      else if (command_args[0] == "join")
      {
        if (command_args.size() != 2)
        {
          std::cerr << "ERR: Invalid usage." << std::endl;
          continue;
        }
        std::string channel_id = command_args[1];
        session->join(channel_id);
      }
      else if (command_args[0] == "rename")
      {
        if (command_args.size() != 2)
        {
          std::cerr << "ERR: Invalid usage." << std::endl;
          continue;
        }

        std::string display_name = command_args[1];
        session->rename(display_name);
      }
      else if (command_args[0] == "help")
      {
        std::cout << help << std::endl;
      }
      else
      {
        std::cerr << "ERR: Invalid command: " << command_args[0] << "." << std::endl;
        std::cout << help << std::endl;
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
        std::cerr << "ERR: Invalid message content. Only characters from ASCII range \\x20-\\x7e are allowed." << std::endl;
        continue;
      }
      session->sendmsg(input);
    }
  }
}
