#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include <string>

class Message;

class MessageFactory
{
public:
  virtual Message *create(const std::string &message) = 0;
};
#endif // MESSAGE_FACTORY_H
