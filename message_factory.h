#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include <string>
#include <memory>

class Message;

class MessageFactory
{
public:
  virtual std::unique_ptr<Message> create(const std::string &message) = 0;
};
#endif // MESSAGE_FACTORY_H
