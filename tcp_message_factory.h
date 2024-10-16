#ifndef TCP_MESSAGE_FACTORY_H
#define TCP_MESSAGE_FACTORY_H

#include "message.h"
#include "message_factory.h"

class TCPMessageFactory : public MessageFactory
{
public:
  std::unique_ptr<Message> create(const std::string &message) override;
};

#endif // TCP_MESSAGE_FACTORY_H
