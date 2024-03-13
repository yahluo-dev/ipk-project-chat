#include "session.h"
#include "exception.h"
#include <iostream>

Session::Session(int _client_socket, std::string _username, std::string _secret, std::string _displayname)
{
  username = _username;
  displayname = _displayname;
  secret = _secret;
  client_socket = _client_socket;
  std::cerr << "DEBUG: Created session." << std::endl;
  server = UDPServer(_client_socket);
  message_id = 1;
}

int Session::sendmsg(std::string _contents)
{
  MsgMessage *message = new MsgMessage(message_id, displayname, _contents);
  server.sendmsg(message);

  message_id++;

  std::cout << "DEBUG: Sending message with contents" << " " << _contents << std::endl;

  ConfirmMessage *expected_confirm = dynamic_cast<ConfirmMessage *>(server.recvmsg());

  if (expected_confirm->code != CODE_CONFIRM ||
      expected_confirm->ref_message_id != message_id)
  {
    std::cerr << "DEBUG: Got something unexpected..." << std::endl;
    std::cout << "Got message with code " << std::to_string(expected_confirm->code) << std::endl;
    std::cout << "Got message with ref_message_id " << std::to_string(expected_confirm->ref_message_id) << std::endl;
  }

  return 1;
}

int Session::join(std::string _channel_id, std::string _displayname)
{
  throw new NotImplemented();
}

int Session::rename(std::string _new_name)
{
  throw new NotImplemented();
}

int Session::auth()
{
  std::cerr << "DEBUG: sending AUTH." << std::endl;
  MessageWithId *message = new AuthMessage(username, secret, displayname, message_id);
  server.send_expect_confirm(message);

  return 0;
}

