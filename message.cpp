#include "message.h"
#include "exception.h"
#include <iostream>
#include <arpa/inet.h>

std::string Message::serialize()
{
  std::cerr << "serialize: Virtual base method called!" << std::endl;
  std::flush(std::cerr);
  std::flush(std::cout);
  throw NotImplemented();
}

Message::~Message() noexcept
{
}

MessageWithId::~MessageWithId() noexcept
{
}

uint16_t MessageWithId::get_message_id()
{
  return message_id;
}

AuthMessage::~AuthMessage() noexcept
{
}
ConfirmMessage::ConfirmMessage(uint16_t _ref_message_id)
{
  code = CODE_CONFIRM;
  ref_message_id = _ref_message_id;
}

std::string Message::make_tcp()
{
  std::cerr << "make_tcp: Virtual base method called!" << std::endl;
  std::flush(std::cout);
  throw NotImplemented();
}

std::string ConfirmMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_CONFIRM);
  uint16_t net_ref_msg_id = ntohs(ref_message_id);
  binary_message += std::string((char *)&net_ref_msg_id, sizeof(uint16_t));

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

std::string AuthMessage::make_tcp()
{
  std::string tcp_message;
  tcp_message += "AUTH";
  tcp_message += " ";
  tcp_message += username;
  tcp_message += " AS ";
  tcp_message += display_name;
  tcp_message += " USING ";
  tcp_message += secret;
  tcp_message += "\r\n";
  return tcp_message;
}

std::string AuthMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_AUTH);
  uint16_t net_msg_id = ntohs(message_id);
  binary_message += std::string((char *)&net_msg_id, sizeof(uint16_t));
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

std::string JoinMessage::make_tcp()
{
  std::string tcp_message;
  tcp_message += "JOIN";
  tcp_message += " ";
  tcp_message += channel_id;
  tcp_message += " AS ";
  tcp_message += display_name;
  tcp_message += "\r\n";
  return tcp_message;
}

std::string JoinMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_JOIN);
  uint16_t net_msg_id = ntohs(message_id);
  binary_message += std::string((char *)&net_msg_id, sizeof(uint16_t));
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

std::string MsgMessage::make_tcp()
{
  std::string tcp_message;
  tcp_message += "MSG FROM";
  tcp_message += " ";
  tcp_message += display_name;
  tcp_message += " IS ";
  tcp_message += message_contents;
  tcp_message += "\r\n";
  return tcp_message;
}

std::string MsgMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_MSG);
  uint16_t net_msg_id = ntohs(message_id);
  binary_message += std::string((char *)&net_msg_id, sizeof(uint16_t));
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

std::string ErrMessage::serialize()
{
  std::string binary_message = std::string(1u, CODE_ERR);
  uint16_t net_msg_id = ntohs(message_id);
  binary_message += std::string((char *)&net_msg_id, sizeof(uint16_t));
  binary_message += display_name;
  binary_message += std::string(1u, '\x00');
  binary_message += message_contents;
  binary_message += std::string(1u, '\x00');

  return binary_message;
}

std::string ErrMessage::make_tcp()
{
  std::string tcp_message;
  tcp_message += "ERR FROM";
  tcp_message += " ";
  tcp_message += display_name;
  tcp_message += " IS ";
  tcp_message += message_contents;
  tcp_message += "\r\n";
  return tcp_message;
}

ByeMessage::ByeMessage(uint16_t _message_id)
{
  code = CODE_BYE;
  message_id = _message_id;
}

std::string ByeMessage::make_tcp()
{
  std::string tcp_message;
  tcp_message += "BYE";
  tcp_message += "\r\n";
  return tcp_message;
}

std::string ByeMessage::serialize()
{
  std::string binary_message = std::string(1u, (char)CODE_BYE);
  uint16_t net_msg_id = htons(message_id);
  binary_message += std::string((char *)&net_msg_id, sizeof(uint16_t));
  return binary_message;
}

UnknownMessage::UnknownMessage(uint8_t _message_code)
{
  code = _message_code;
}

MessageWithId::MessageWithId(message_code_t _code, int _message_id)
{
  code = _code;
  message_id = _message_id;
}

MessageWithId::MessageWithId()
{
  message_id = -1;
}