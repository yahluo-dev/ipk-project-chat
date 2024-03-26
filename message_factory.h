#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include "abstract_factory.h"
#include "message.h"
#include <vector>

class MessageFactory : public AbstractFactory
{
  private:
    std::vector<std::string> parse_null_terminated_data(char *raw_data, int n_fields);
  public:
    Message *create(std::string message);
};

#endif // MESSAGE_FACTORY_H
