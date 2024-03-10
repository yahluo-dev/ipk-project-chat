#include "message_factory.h"
#include <vector>

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

Message MessageFactory::create(std::string message)
{
  message_t *generic_message = (message_t *)message.data();

  switch(generic_message->code)
  {
    case CODE_CONFIRM:
    {
      confirm_message_t *msg = (confirm_message_t *)generic_message;
      return ConfirmMessage(msg->ref_message_id);
    }
    case CODE_REPLY:
    {
      reply_message_t *msg = (reply_message_t *)generic_message;
      auto data_fields = parse_null_terminated_data(msg->message_contents, 1);
      std::string message_contents = data_fields[0];
      return ReplyMessage(msg->message_id, msg->result, msg->message_id,
          message_contents);
    }
    case CODE_AUTH:
    {
      auth_message_t *msg = (auth_message_t *)generic_message;
      auto data_fields = parse_null_terminated_data(msg->data, 3);
      std::string username = data_fields[0];
      std::string displayname = data_fields[1];
      std::string secret = data_fields[2];
      return AuthMessage(username, secret, displayname, msg->message_id);
    }
    case CODE_JOIN:
    {
      join_message_t *msg = (join_message_t *)generic_message;
      auto data_fields = parse_null_terminated_data(msg->data, 2);
      std::string channel_id = data_fields[0];
      std::string displayname = data_fields[1];
      return JoinMessage(msg->message_id, channel_id, displayname);
    }
    case CODE_MSG:
    {
      msg_message_t *msg = (msg_message_t *)generic_message;
      auto data_fields = parse_null_terminated_data(msg->data, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return MsgMessage(msg->message_id, displayname, message_contents);
    }
    case CODE_ERR:
    {
      err_message_t *msg = (err_message_t *)generic_message;
      auto data_fields = parse_null_terminated_data(msg->data, 2);
      std::string displayname = data_fields[0];
      std::string message_contents = data_fields[1];
      return ErrMessage(msg->message_id, displayname, message_contents);
    }
    case CODE_BYE:
    {
      bye_message_t *msg = (bye_message_t *)generic_message;
      return ByeMessage(msg->message_id);
    }
  }
}
