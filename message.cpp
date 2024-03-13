#include "message.h"
#include <cstring>
#include "exception.h"
#include <arpa/inet.h>
#include <iostream>

std::string Message::serialize()
{
  std::cerr << "Virtual base method called!" << std::endl;
  throw new NotImplemented();
}

ConfirmMessage::ConfirmMessage(uint16_t _ref_message_id)
{
  code = CODE_CONFIRM;
  ref_message_id = _ref_message_id;
}
std::string ConfirmMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_CONFIRM);
  binary_message += std::string((char *)&ref_message_id, sizeof(uint16_t));

  return binary_message;
}

AuthMessage::AuthMessage(std::string _username, 
    std::string _secret, std::string _display_name, uint16_t _message_id)
{
  code = CODE_AUTH;
  username = _username;
  secret = _secret;
  display_name = _display_name;
  message_id = _message_id;
}

std::string AuthMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_AUTH);
  binary_message += std::string((char *)&message_id, sizeof(uint16_t));
  binary_message += username;
  binary_message += std::string(1u, '\x00');
  binary_message += display_name;
  binary_message += std::string(1u, '\x00');
  binary_message += secret;
  binary_message += std::string(1u, '\x00');

  return binary_message;
}

ReplyMessage::ReplyMessage(uint16_t _message_id, uint8_t _result, uint16_t _ref_message_id,
    std::string _message_contents)
{
  code = CODE_REPLY;
  message_id = _message_id;
  result = _result;
  ref_message_id = _ref_message_id;
  message_contents = _message_contents;
}

JoinMessage::JoinMessage(uint16_t _message_id, std::string _channel_id,
    std::string _display_name)
{
  code = CODE_JOIN;
  message_id = _message_id;
  channel_id = _channel_id;
  display_name = _display_name;
}

std::string JoinMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_JOIN);
  binary_message += std::string((char *)&message_id, sizeof(uint16_t));
  binary_message += channel_id;
  binary_message += std::string(1u, '\x00');
  binary_message += display_name;
  binary_message += std::string(1u, '\x00');

  return binary_message;
}

MsgMessage::MsgMessage(uint16_t _message_id, std::string _display_name,
    std::string _message_contents)
{
  code = CODE_MSG;
  message_id = _message_id;
  display_name = _display_name;
  message_contents = _message_contents;
}

std::string MsgMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_MSG);
  binary_message += std::string((char *)&message_id, sizeof(uint16_t));
  binary_message += display_name;
  binary_message += std::string(1u, '\x00');
  binary_message += message_contents;
  binary_message += std::string(1u, '\x00');

  return binary_message;
}

ErrMessage::ErrMessage(uint16_t _message_id, std::string _display_name,
    std::string _message_contents)
{
  code = CODE_ERR;
  message_id = _message_id;
  display_name = _display_name;
  message_contents = _message_contents;
}

ByeMessage::ByeMessage(uint16_t _message_id)
{
  code = CODE_BYE;
  message_id = _message_id;
}

