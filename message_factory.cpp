#include "message_factory.h"
#include <vector>
#include <stdexcept>
#include <iostream>

// TODO: There will be received and sent messages.
// For sent ones it would be good to specify only the parameters and some class would 
// serialize it.
// For received messages I could create messages from a generic message struct, like I'm doing here.
// Use templates?
// serialize() method on messages?

/**
 * Create a new Message object from a generic struct message_t.
 */

std::vector<std::string> parse_null_terminated_data(char *raw_data, int n_fields)
{
  // FIXME: Might go beyond the packet end! Use std::string?

  // Entering the realm of pointer arithmetic

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
  size_t message_size = message.size();

  // Packet code
  code = *(uint8_t *)generic_message;
  generic_message++;

  switch(code)
  {
    case CODE_CONFIRM:
    {
      uint16_t ref_message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);
      std::cerr << "DEBUG message_factory: ref_message_id "
        << std::to_string(ref_message_id) << std::endl;

      return new ConfirmMessage(ref_message_id);
    }
    case CODE_REPLY:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);
      uint8_t result = *(uint8_t *)generic_message;
      generic_message += sizeof(uint8_t);
      uint16_t ref_message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);
      auto data_fields = parse_null_terminated_data(generic_message, 1);
      std::string message_contents = data_fields[0];

      return new ReplyMessage(message_id, result, ref_message_id,
          message_contents);
    }
    case CODE_MSG:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);

      auto data_fields = parse_null_terminated_data(generic_message, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return new MsgMessage(message_id, displayname, message_contents);
    }
    case CODE_ERR:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);

      auto data_fields = parse_null_terminated_data(generic_message, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return new ErrMessage(message_id, displayname, message_contents);
    }
    case CODE_BYE:
    {
      uint16_t message_id = *(uint16_t *)generic_message;
      generic_message += sizeof(uint16_t);
      return new ByeMessage(message_id);
    }
    default:
    {
      throw new std::invalid_argument("Packet with unexpected code received.");
    }
  }
}
