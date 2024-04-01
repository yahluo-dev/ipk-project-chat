#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <stdexcept>

class NotImplemented : public std::logic_error
{
  public:
    NotImplemented() : std::logic_error("Function not implemented yet!") {};
};

class ConnectionFailed: public std::runtime_error
{
public:
  ConnectionFailed() : std::runtime_error("Server did not respond!") {};
};

class BadConfirm : public std::runtime_error
{
public:
  BadConfirm() : std::runtime_error("Got unexpected ref_message_id!") {};
};

#endif // EXCEPTION_H
