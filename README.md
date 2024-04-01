# IPK-Project-1

## Introduction

The client for **IPK-Chat** connects to the server via the specified protocol and allows the user to authenticate, send and receive messages and to join channels.

## Usage

### Command line arguments

The two required arguments are HOSTNAME and PROTOCOL. The user can also specify the port (**-p**, default is 4567), UDP confirmation timeout in milliseconds (**-d**, default is 250), and the maximum number of UDP retransmissions (**-r**, default is 3). 

UDP-related options will be ignored when -t tcp is specified. **-h** shows a help message and exits.

### Commands

Commands are identified by the "/" in the beginning. The following commands are available:

- **/auth** *{username}* *{secret}* *{display_name}* - Attempt to authenticate at the remote server. The value in the DISPLAY_NAME option will be shown to all the other users when a user sends a message to a channel. If the authentication is successful, there will be a *Success: {message}* message and the user will be allowed to send messages to the channel. Otherwise, a *Failure: {message}* will appear.
- **/join** *{channel_id}* - Join the channel identified by CHANNEL_ID at the remote server.
- **/rename** *{display_name}* - Change the display name shown to other users.
- **/help** - Show the help message.

Text not preceeded by a "/" is interpreted as a MESSAGE to the current channel.

*{username}* can only contain alphanumeric characters and "-" characters, must be of length between 1 and 20 characters.
*{secret}* can only contain alphanumeric characters and "-" characters, must be of length between 1 and 128 characters.
*{display_name}* can only contain printable characters (ascii range "!" to "~") and must be of length between 1 and 20 characters.
*{message}* can only contain printable characters (ascii range "!" to "~") and spaces and must be of length between 1 and 1400 characters.

## Testing

There are unit tests as well as system tests are available in the *tests/* subdirectory. The command **make test** will run the system tests, and **make unit-test** will run the unit tests.

The unit tests focus on parts responsible for serializing and deserializing messages, as they are they are the sections that give semantics to accepted binary and text messages and so, are core of the protocol implementation.

The system tests cover some of the most common use cases of the program, running it in UDP and in TCP mode, authenticating, sending messages, renaming and joining channels. They also check that the program gracefully closes the connection and exits when the user interrupts it or when the standard input is closed.

## Structure

The program is written in the C++ language and is structured into source files and headers. Following is the list of all files of the program:

- *message_factory.cpp, message_factory.h* - Abstract message factory class for decoding messages.
- *tcp_message_factory.cpp, tcp_message_factory.h* - TCP message factory class for decoding TCP messages.
- *udp_message_factory.cpp, udp_message_factory.h* - UDP message factory class for decoding UDP messages.
- *client.cpp, client.h* - UDP message factory class for decoding UDP messages.
- *exception.cpp, exception.h* - Custom exception classes used by the application.
- *main.cpp, main.h* - Contains the main function handling CLI arguments and creating a client.
- *message.cpp, message.h* - Implements various message classes.
- *session.cpp, session.h* - Session abstract class, interface between the user frontend, sender and the receiver.
- *tcp_session.cpp, tcp_session.h* - Concrete TCP session class implementation, does not have message id validation, unlike the UDP counterpart.
- *udp_session.cpp, udp_session.h* - Concrete UDP session class implementation.
- *sender.h* - Abstract sender class for sending messages over TCP and UDP.
- *tcp_sender.cpp, tcp_sender.h* - Concrete TCP sender class for sending messages over TCP.
- *udp_sender.cpp, udp_sender.h* - Concrete UDP sender class for sending messages over UDP.
- *.cpp, udp_sender.h* - Concrete UDP sender class for sending messages over UDP.
- *tcp_receiver.cpp, tcp_reciever.h* - Concrete TCP receiver class for receiving and decoding TCP messages.
- *udp_receiver.cpp, udp_reciever.h* - Concrete UDP receiver class for UDP messages.

### Tests

The **tests/** directory contains the following files:

- *udp_message_factory_tests.cpp* - Tests for the UDP message factory decoding incoming UDP messages.
- *udp_message_serialize_tests.cpp* - UDP message serialize() method tests
- *tcp_message_factory_tests.cpp* - Tests for the TCP message factory
- *tcp_message_make_tcp_tests.cpp* - Tests for the make_tcp method of messages


### Class diagram

![alt text](./doc/class.mmd.png)
*UML Class diagram describing the design of the client*
