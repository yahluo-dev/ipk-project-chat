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

class NotAuthenticated : public std::runtime_error
{
public:
  NotAuthenticated() : std::runtime_error("Not authenticated.") {};
};

class NotInChannel : public std::runtime_error
{
public:
  NotInChannel() : std::runtime_error("Not in channel.") {};
};

class ReplEof : public std::runtime_error
{
public:
  ReplEof() : std::runtime_error("EOF") {};
};

class UnexpectedMessage : public std::runtime_error
{
public:
  UnexpectedMessage() : std::runtime_error("Got unexpected message from server") {};
};

class BadConfirm : public std::runtime_error
{
public:
  BadConfirm() : std::runtime_error("Got unexpected ref_message_id!") {};
};

#endif // EXCEPTION_H
