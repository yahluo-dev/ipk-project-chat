#ifndef ABSTACT_FACTORY_H
#define ABSTACT_FACTORY_H

#include <string>

class Message;

class MessageFactory
{
public:
  virtual Message *create(const std::string &message) = 0;
};
#endif // ABSTACT_FACTORY_H
