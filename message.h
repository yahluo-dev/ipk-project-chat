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
  CODE_BYE = 0xFF
};


class Message
{
  protected:
  public:
    uint8_t code;
    virtual std::string serialize();
};

class ConfirmMessage : public Message
{
  private:
  public:
    uint16_t ref_message_id;
    ConfirmMessage(uint16_t _ref_message_id);
    std::string serialize() override;
};

class MessageWithId : public Message
{
  private:
  public:
    uint16_t message_id;
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
};

class JoinMessage : public MessageWithId
{
  private:
  public:
    std::string channel_id;
    std::string display_name;
    JoinMessage(uint16_t _message_id, std::string _channel_id, std::string _display_name);
    std::string serialize() override;
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
};

class ErrMessage : public MessageWithId
{
  private:
  public:
    std::string display_name;
    std::string message_contents;
    ErrMessage(uint16_t _message_id, std::string _display_name,
        std::string _message_contents);
};

class ByeMessage : public MessageWithId
{
  private:
  public:
    ByeMessage(uint16_t _message_id);
};

#endif // MESSAGE_H
