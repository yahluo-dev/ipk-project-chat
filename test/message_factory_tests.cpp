#include <gtest/gtest.h>
#include "../message_factory.h"
#include <string>

TEST(MsgFactoryTests, CreateConfirm)
{
  std::string binary_message = std::string("\x00", 1); // Have to specify size 
                                                       // if contains \0
  binary_message += "\xde\xad";

  MessageFactory factory;

  auto message = dynamic_cast<ConfirmMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->get_ref_message_id(), 57005);
}

TEST(MsgFactoryTests, CreateReply)
{
  std::string binary_message="\x01";  // code
  binary_message += "\xde\xad";       // message_id
  binary_message += "\x01";           // result
  binary_message += "\xbe\xef";       // ref_messageid
  binary_message += std::string("Hiiii\n\nhello\tnice to meet you\x00", 30);

  MessageFactory factory;

  auto message = dynamic_cast<ReplyMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->get_message_id(), 57005); // 0xdead
  EXPECT_EQ(message->get_result(), 1);
  EXPECT_EQ(message->get_ref_message_id(), 48879); // 0xbeef
  EXPECT_EQ(message->get_contents(), "Hiiii\n\nhello\tnice to meet you");
}

TEST(MsgFactoryTests, CreateMsg)
{
  std::string binary_message="\x04"; // code
  binary_message += "\xde\xad"; // message_id
  binary_message += std::string("i_love_my_display_name\x00", 23); // display_name
  binary_message += std::string("where are the stickers?\x00", 24); // message_contents

  MessageFactory factory;

  auto message = dynamic_cast<MsgMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->get_message_id(), 0xdead); // 0xdead
  EXPECT_EQ(message->get_display_name(), "i_love_my_display_name");
  EXPECT_EQ(message->get_contents(), "where are the stickers?");
}

TEST(MsgFactoryTests, CreateErr)
{
  std::string binary_message="\xfe"; // code
  binary_message += "\xbe\xef"; // message_id
  binary_message += std::string("i_love_my_display_name\x00", 23); // display_name
  binary_message += std::string("where are the stickers?\x00", 24); // message_contents

  MessageFactory factory;

  auto message = dynamic_cast<ErrMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->get_message_id(), 0xbeef);
  EXPECT_EQ(message->get_display_name(), "i_love_my_display_name");
  EXPECT_EQ(message->get_contents(), "where are the stickers?");
}
