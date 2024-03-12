#include "message.h"
#include <cstring>
#include "exception.h"

std::string Message::serialize()
{
  throw new NotImplemented();
}

ConfirmMessage::ConfirmMessage(uint16_t _ref_message_id)
{
  code = CODE_CONFIRM;
  ref_message_id = _ref_message_id;
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

MsgMessage::MsgMessage(uint16_t _message_id, std::string _display_name,
    std::string _message_contents)
{
  code = CODE_MSG;
  message_id = _message_id;
  display_name = _display_name;
  message_contents = _message_contents;
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

std::string AuthMessage::serialize()
{
  throw new NotImplemented();
}
