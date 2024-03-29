#ifndef TCP_MESSAGE_FACTORY_H
#define TCP_MESSAGE_FACTORY_H

#include "message.h"
#include "abstract_factory.h"

class TCPMessageFactory : public AbstractFactory
{
private:
public:
  Message *create(const std::string &message);
};

#endif // TCP_MESSAGE_FACTORY_H
