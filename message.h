#ifndef MESSAGE_H
#define MESSAGE_H

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
  CODE_BYE = 0xFF,
  CODE_UNKNOWN = 0xEE
};


class Message
{
  protected:
  public:
    uint8_t code;
    virtual std::string serialize();
    virtual std::string make_tcp();
    virtual ~Message();
};

class ConfirmMessage : public Message
{
  private:
  public:
    uint16_t ref_message_id;
    explicit ConfirmMessage(uint16_t _ref_message_id);
    std::string serialize() override;
};

class MessageWithId : public Message
{
  protected:
    uint16_t message_id;
  public:
    uint16_t get_message_id();
    MessageWithId();
    MessageWithId(message_code_t _code, int _message_id);
    ~MessageWithId() override;
};

class ReplyMessage : public MessageWithId
{
  private:
  public:
    uint8_t result;
    uint16_t ref_message_id;
    std::string message_contents;
    ReplyMessage(uint16_t _message_id, uint8_t _result, uint16_t _ref_message_id,
        std::string _message_contents);
};

class AuthMessage : public MessageWithId
{
  private:
  public:
    std::string username;
    std::string secret;
    std::string display_name;
    AuthMessage(std::string _username, 
        std::string _secret, std::string _display_name, uint16_t _message_id);
    std::string serialize() override;
    ~AuthMessage() override;
    std::string make_tcp();
};

class JoinMessage : public MessageWithId
{
  private:
  public:
    std::string channel_id;
    std::string display_name;
    JoinMessage(uint16_t _message_id, std::string _channel_id, std::string _display_name);
    std::string serialize() override;
    std::string make_tcp();
};

class MsgMessage : public MessageWithId
{
  private:
  public:
    std::string display_name;
    std::string message_contents;
    MsgMessage(uint16_t _message_id, std::string _display_name,
        std::string _message_contents);
    std::string serialize() override;
    std::string make_tcp();
};

class ErrMessage : public MessageWithId
{
  private:
  public:
    std::string display_name;
    std::string message_contents;
    ErrMessage(uint16_t _message_id, std::string _display_name,
        std::string _message_contents);
    std::string make_tcp();
};

class ByeMessage : public MessageWithId
{
  private:
  public:
    explicit ByeMessage(uint16_t _message_id);
    std::string serialize() override;
    std::string make_tcp();
};

class UnknownMessage : public Message
{
  public:
    explicit UnknownMessage(uint8_t code);
};

#endif // MESSAGE_H
