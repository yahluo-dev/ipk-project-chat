#include <gtest/gtest.h>
#include "../tcp_message_factory.h"
#include "string"

TEST(TCPMessageFactoryTests, CreateReply)
{
  std::string message_string = std::string("REPLY OK IS abc\r\n");

  TCPMessageFactory factory;

  auto message = dynamic_cast<ReplyMessage *>(factory.create(message_string));

  EXPECT_EQ(message->get_result(), 1);
  EXPECT_EQ(message->get_contents(), "abc");
}

TEST(TCPMessageFactoryTests, CreateMsg)
{
  std::string message_string = std::string("MSG FROM test-user IS Hii\r\n");

  TCPMessageFactory factory;

  auto message = dynamic_cast<MsgMessage *>(factory.create(message_string));

  EXPECT_EQ(message->get_display_name(), "test-user");
  EXPECT_EQ(message->get_contents(), "Hii");
}

TEST(TCPMessageFactoryTests, CreateErr)
{
  std::string message_string = std::string("ERR FROM Server IS Invalid message.\r\n");

  TCPMessageFactory factory;

  auto message = dynamic_cast<ErrMessage *>(factory.create(message_string));

  EXPECT_EQ(message->get_display_name(), "Server");
  EXPECT_EQ(message->get_contents(), "Invalid message.");
}
