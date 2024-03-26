#ifndef SESSION_H
#define SESSION_H

#include <string>
#include <cstdint>
#include <netinet/in.h>
#include "server.h"
#include "message.h"
#include <vector>
#include <thread>
#include "receiver.h"

class UDPReceiver;
class UDPSender;

enum session_state_t
{
  STATE_START,
  STATE_AUTH,
  STATE_JOIN,
  STATE_OPEN,
  STATE_ERROR,
  STATE_END
};

class Session
{
  private:
    int client_socket;
    uint16_t message_id;
    std::string username, secret, displayname;
    UDPSender *sender;
    static volatile session_state_t state;
    std::chrono::milliseconds timeout;
    UDPReceiver *receiver;
    std::jthread receiving_thread;
    static std::vector<Message *> inbox;

  public:
    Session(int _client_socket, unsigned int max_retr, std::chrono::milliseconds timeout);
    virtual int sendmsg(const std::string &contents);
    virtual int join(const std::string &channel_id, const std::string &displayname);
    virtual int rename(const std::string &new_name);
    virtual int auth(const std::string &_username, const std::string &_secret, const std::string &_displayname);

    virtual session_state_t get_state();

    virtual void notify_incoming(Message *message);
};
#endif // SESSION_H
