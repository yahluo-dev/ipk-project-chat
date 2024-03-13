#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include "abstract_factory.h"
#include "message.h"

class MessageFactory : public AbstractFactory
{
  public:
    Message *create(std::string message);
};

#endif // MESSAGE_FACTORY_H
