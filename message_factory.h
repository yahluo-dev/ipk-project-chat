#include "abstract_factory.h"
#include "message.h"

class MessageFactory : public AbstractFactory
{
  public:
    Message *create(std::string message);
};
