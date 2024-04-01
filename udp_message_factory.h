#ifndef MESSAGE_FACTORY_H
#define MESSAGE_FACTORY_H

#include "message_factory.h"
#include "message.h"
#include <vector>

class UDPMessageFactory : public MessageFactory
{
  private:
    std::vector<std::string> parse_null_terminated_data(char *raw_data, int n_fields);
  public:
    Message *create(const std::string &message) override;
};

#endif // MESSAGE_FACTORY_H
