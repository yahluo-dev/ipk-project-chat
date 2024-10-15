#ifndef UDP_MESSAGE_FACTORY_H
#define UDP_MESSAGE_FACTORY_H

#include "message.h"
#include "message_factory.h"
#include <vector>

class UDPMessageFactory : public MessageFactory
{
  private:
    std::vector<std::string> parse_null_terminated_data(char *raw_data, int n_fields);
  public:
    Message *create(const std::string &message) override;
};

#endif // UDP_MESSAGE_FACTORY_H
