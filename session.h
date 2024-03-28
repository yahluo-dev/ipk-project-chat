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
#include <exception>
#include <netdb.h>

class UDPReceiver;
class UDPSender;

enum session_state_t
{
  STATE_START,
  STATE_AUTH,
  STATE_JOIN,
  STATE_OPEN,
  STATE_ERROR,
  STATE_END,
  STATE_INTERNAL_ERROR
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
    std::exception_ptr receiver_ex;
    struct addrinfo *server_addrinfo;
    std::string hostname;
    int max_retr;
  public:
    Session(const std::string &hostname, const std::string& port, unsigned int max_retr, std::chrono::milliseconds timeout);
    ~Session();
    virtual int sendmsg(const std::string &contents);
    virtual int join(const std::string &channel_id, const std::string &displayname);
    virtual int rename(const std::string &new_name);
    virtual int auth(const std::string &_username, const std::string &_secret, const std::string &_displayname);
    virtual void set_receiver_ex();
    virtual void update_port(const std::string &port);
    virtual void bye();

    virtual session_state_t get_state();

    virtual void notify_incoming(Message *message);
};
#endif // SESSION_H
