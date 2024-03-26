#ifndef SERVER_H
#define SERVER_H

#include "message.h"
#include <sys/socket.h>
#include <vector>
#include <thread>
#include "sender.h"

class UDPSender;

class UDPServer
{
  private:
    int sock;
    int max_retr;
    std::vector<Message *> inbox;
    std::vector<Message *> confirms;
    std::jthread receiving_thread;
    ConfirmMessage *check_got_confirm(uint16_t ref_message_id);
    UDPSender *sender;
    std::chrono::milliseconds timeout;
  public:
    UDPServer();
    UDPServer(int _sock, int max_retr, std::chrono::milliseconds _timeout);
    void send_msg(Message *msg);
    void send_expect_confirm(MessageWithId *msg);
    void notify_incoming(Message *msg);
    void notify_send_success();
    Message *get_msg();
};

#endif // SERVER_H
