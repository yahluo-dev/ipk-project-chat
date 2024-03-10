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
  server = new UDPServer(_client_socket);
  message_id = 1;
}

int Session::sendmsg(std::string _contents)
{
  throw new NotImplemented();
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
  AuthMessage *message = new AuthMessage(username, secret, displayname, message_id);
  server->send_msg(message);
}

