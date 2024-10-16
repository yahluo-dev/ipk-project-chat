#include "udp_message_factory.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>

/**
 * Create a new Message object from a generic struct message_t.
 */

std::vector<std::string> UDPMessageFactory::parse_null_terminated_data(char *raw_data, int n_fields)
{
  std::vector<std::string> vec;

  for (int i = 0; i < n_fields; i++)
  {
    std::string new_field = raw_data;
    vec.push_back(new_field);
    raw_data += new_field.size() + 1; // Skip '\0' as well
  }

  return vec;
}

std::unique_ptr<Message> UDPMessageFactory::create(const std::string &message)
{
  uint8_t code;
  char *generic_message = (char *)message.data();

  // Packet code
  code = *(uint8_t *)generic_message;
  generic_message++;

  switch(code)
  {
    case CODE_CONFIRM:
    {
      uint16_t ref_message_id = *(uint16_t *)generic_message;
      ref_message_id = ntohs(ref_message_id);
      generic_message += sizeof(uint16_t);
      return std::make_unique<ConfirmMessage>(ref_message_id);
    }
    case CODE_REPLY:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);
      uint8_t result = *(uint8_t *)generic_message;
      generic_message += sizeof(uint8_t);
      uint16_t ref_message_id = *(uint16_t *)generic_message;
      ref_message_id = ntohs(ref_message_id);
      generic_message += sizeof(uint16_t);
      auto data_fields = parse_null_terminated_data(generic_message, 1);
      std::string message_contents = data_fields[0];

      return std::make_unique<ReplyMessage>(message_id, result, ref_message_id,
          message_contents);
    }
    case CODE_MSG:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);
      auto data_fields = parse_null_terminated_data(generic_message, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return std::make_unique<MsgMessage>(message_id, displayname, message_contents);
    }
    case CODE_ERR:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);

      auto data_fields = parse_null_terminated_data(generic_message, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return std::make_unique<ErrMessage>(message_id, displayname, message_contents);
    }
    case CODE_BYE:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);
      return std::make_unique<ByeMessage>(message_id);
    }
    default:
    {
      throw std::invalid_argument("Packet with unexpected code received.");
      return std::make_unique<UnknownMessage>(code);
    }
  }
}
