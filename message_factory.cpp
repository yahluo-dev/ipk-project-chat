#include "message_factory.h"
#include <vector>
#include <stdexcept>
#include <iostream>
#include <arpa/inet.h>

// TODO: Fix endianness of numbers (Must be BE when transmitting)

/**
 * Create a new Message object from a generic struct message_t.
 */

std::vector<std::string> MessageFactory::parse_null_terminated_data(char *raw_data, int n_fields)
{
  // FIXME: Might go beyond the packet end! Use std::string?

  std::vector<std::string> vec;

  for (int i = 0; i < n_fields; i++)
  {
    std::string new_field = raw_data;
    vec.push_back(new_field);
    raw_data += new_field.size() + 1; // Skip '\0' as well
  }

  return vec;
}

Message *MessageFactory::create(std::string message)
{
  uint8_t code;
  char *generic_message = message.data();
  // size_t message_size = message.size();

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
      return new ConfirmMessage(ref_message_id);
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

      return new ReplyMessage(message_id, result, ref_message_id,
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
      return new MsgMessage(message_id, displayname, message_contents);
    }
    case CODE_ERR:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);

      auto data_fields = parse_null_terminated_data(generic_message, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return new ErrMessage(message_id, displayname, message_contents);
    }
    case CODE_BYE:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      message_id = ntohs(message_id);
      generic_message += sizeof(uint16_t);
      return new ByeMessage(message_id);
    }
    default:
    {
      throw std::invalid_argument("Packet with unexpected code received.");
      //return new UnknownMessage(code);
    }
  }
}
