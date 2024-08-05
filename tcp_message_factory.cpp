#include "tcp_message_factory.h"
#include <regex>

std::regex msg_regex("MSG FROM ([a-zA-Z0-9-]{1,20}) IS ([ -~]{1,1400})\r\n", std::regex_constants::ECMAScript);
std::regex err_regex("ERR FROM ([a-zA-Z0-9-]{1,20}) IS ([ -~]{1,1400})\r\n", std::regex_constants::ECMAScript);
std::regex reply_regex("REPLY (OK|NOK) IS ([ -~]{1,1400})\r\n", std::regex_constants::ECMAScript);
std::regex bye_regex("BYE\r\n", std::regex_constants::ECMAScript);

Message *TCPMessageFactory::create(const std::string &message)
{
  std::smatch matches;
  if (!std::regex_match(message, std::regex("[^\r\n]+\r\n")))
  {
    return new UnknownMessage(CODE_UNKNOWN);
  }

  if (std::regex_match(message, matches, msg_regex))
  {
    std::string display_name = matches[1];
    std::string message_contents = matches[2];
    auto parsed_message = new MsgMessage(0, display_name, message_contents);
    return parsed_message;
  }
  else if (std::regex_match(message, matches, err_regex))
  {
    std::string display_name = matches[1];
    std::string message_contents = matches[2];
    auto parsed_message = new ErrMessage(0, display_name, message_contents);
    return parsed_message;
  }
  else if (std::regex_match(message, matches, reply_regex))
  {
    std::string result = matches[1];
    std::string message_contents = matches[2];
    auto parsed_message = new ReplyMessage(0,
                                           (result == "OK" ? 1 : 0),
                                           0, message_contents);
    return parsed_message;
  }
  else if (std::regex_match(message, matches, bye_regex))
  {
    auto parsed_message = new ByeMessage(0);
    return parsed_message;
  }
  else
  {
    auto parsed_message = new UnknownMessage(CODE_UNKNOWN);
    return parsed_message;
  }
}
