#include <cstdint>
#include <string>

enum message_code_t
{
  CODE_CONFIRM = 0x00,
  CODE_REPLY = 0x01,
  CODE_AUTH = 0x02,
  CODE_JOIN = 0x03,
  CODE_MSG = 0x04,
  CODE_ERR = 0xFE,
  CODE_BYE = 0xFF
};

// Generic message struct
typedef struct
{
  uint8_t code;
  char message_data[1]; // This is actually used as a flexible member,
                        // but the C++ standard doesn't allow them 
                        // and the compiler will complain, hence [1].
                        // FIXME: think of something better.
} message_t;

typedef struct
{
  uint8_t code;
  uint16_t ref_message_id;
} confirm_message_t;

typedef struct
{
  uint8_t code;
  uint16_t message_id;
  uint8_t result;
  uint16_t ref_message_id;
  char message_contents[1];

} reply_message_t;

typedef struct
{
  uint8_t code;
  uint16_t message_id;
  char data[1];

} auth_message_t;


// Structs are going to be the same
typedef auth_message_t join_message_t;
typedef auth_message_t msg_message_t;
typedef auth_message_t err_message_t;

typedef struct
{
  uint8_t code;
  uint16_t message_id;
} bye_message_t;


class Message
{
  protected:
    uint8_t code;
  public:
    virtual std::string serialize();
};

class ConfirmMessage : public Message
{
  private:
  public:
    uint16_t ref_message_id;
    ConfirmMessage(uint16_t _ref_message_id);
    std::string serialize();
};

class ReplyMessage : public Message
{
  private:
  public:
    uint16_t message_id;
    uint8_t result;
    uint16_t ref_message_id;
    std::string message_contents;
    ReplyMessage(uint16_t _message_id, uint8_t _result, uint16_t _ref_message_id,
        std::string _message_contents);
};

class AuthMessage : public Message
{
  private:
  public:
    std::string username;
    std::string secret;
    std::string display_name;
    uint16_t message_id;
    AuthMessage(std::string _username, 
        std::string _secret, std::string _display_name, uint16_t _message_id);
    std::string serialize();
};

class JoinMessage : public Message
{
  private:
  public:
    uint16_t message_id;
    std::string channel_id;
    std::string display_name;
    JoinMessage(uint16_t _message_id, std::string _channel_id, std::string _display_name);
};

class MsgMessage : public Message
{
  private:
  public:
    uint16_t message_id;
    std::string display_name;
    std::string message_contents;
    MsgMessage(uint16_t _message_id, std::string _display_name,
        std::string _message_contents);
};

class ErrMessage : public Message
{
  private:
  public:
    uint16_t message_id;
    std::string display_name;
    std::string message_contents;
    ErrMessage(uint16_t _message_id, std::string _display_name,
        std::string _message_contents);
};

class ByeMessage : public Message
{
  private:
  public:
    uint16_t message_id;
    ByeMessage(uint16_t _message_id);
};
