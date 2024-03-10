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

// Flexible members are not a part of any C++ standard,
// this is not standard C++ code.
extern "C" {

// Generic message struct
typedef struct
{
  uint8_t code;
  char message_data[];
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
  char message_contents[];

} reply_message_t;

typedef struct
{
  uint8_t code;
  uint16_t message_id;
  char data[];

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

}

class Message
{
  protected:
    uint8_t code;
  public:
    std::string serialize();
};

class ConfirmMessage : public Message
{
  private:
    uint16_t ref_messageid;
  public:
    ConfirmMessage(uint16_t _ref_messageid);
};

class ReplyMessage : public Message
{
  private:
    uint16_t message_id;
    uint8_t result;
    uint16_t ref_messageid;
    std::string message_contents;
  public:
    ReplyMessage(uint16_t _message_id, uint8_t _result, uint16_t _ref_messageid,
        std::string _message_contents);
};

class AuthMessage : public Message
{
  private:
    std::string username;
    std::string secret;
    std::string displayname;
    uint16_t message_id;
  public:
    AuthMessage(std::string _username, 
        std::string _secret, std::string _displayname, uint16_t _message_id);
    std::string serialize();
};

class JoinMessage : public Message
{
  private:
    uint16_t message_id;
    std::string channel_id;
    std::string displayname;
  public:
    JoinMessage(uint16_t _message_id, std::string _channel_id, std::string _displayname);
};

class MsgMessage : public Message
{
  private:
    uint16_t message_id;
    std::string displayname;
    std::string message_contents;
  public:
    MsgMessage(uint16_t _message_id, std::string _displayname,
        std::string _message_contents);
};

class ErrMessage : public Message
{
  private:
    uint16_t message_id;
    std::string displayname;
    std::string message_contents;
  public:
    ErrMessage(uint16_t _message_id, std::string _displayname,
        std::string _message_contents);
};

class ByeMessage : public Message
{
  private:
    uint16_t message_id;
  public:
    ByeMessage(uint16_t _message_id);
};
