#include <gtest/gtest.h>
#include "../message_factory.h"
#include <string>

// FIXME: Is the byte order correct?

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(MsgFactoryTests, CreateConfirm)
{
  std::string binary_message = std::string("\x00", 1); // Have to specify size 
                                                       // if contains \0
  binary_message += "\xad\xde";

  MessageFactory factory;

  ConfirmMessage *message = dynamic_cast<ConfirmMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->ref_message_id, 57005);
}

TEST(MsgFactoryTests, CreateReply)
{
  std::string binary_message="\x01";  // code
  binary_message += "\xad\xde";       // message_id
  binary_message += "\x01";           // result
  binary_message += "\xef\xbe";       // ref_messageid
  binary_message += std::string("Hiiii\n\nhello\tnice to meet you\x00", 30);

  MessageFactory factory;

  ReplyMessage *message = dynamic_cast<ReplyMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->message_id, 57005); // 0xdead
  EXPECT_EQ(message->result, 1);
  EXPECT_EQ(message->ref_message_id, 48879); // 0xbeef
  EXPECT_EQ(message->message_contents, "Hiiii\n\nhello\tnice to meet you");
}

TEST(MsgFactoryTests, CreateMsg)
{
  std::string binary_message="\x04"; // code
  binary_message += "\xad\xde"; // message_id
  binary_message += std::string("i_love_my_display_name\x00", 23); // display_name
  binary_message += std::string("where are the stickers?\x00", 24); // message_contents

  MessageFactory factory;

  MsgMessage *message = dynamic_cast<MsgMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->message_id, 57005); // 0xdead
  EXPECT_EQ(message->display_name, "i_love_my_display_name");
  EXPECT_EQ(message->message_contents, "where are the stickers?");
}

TEST(MsgFactoryTests, CreateErr)
{
  std::string binary_message="\xfe"; // code
  binary_message += "\xad\xde"; // message_id
  binary_message += std::string("i_love_my_display_name\x00", 23); // display_name
  binary_message += std::string("where are the stickers?\x00", 24); // message_contents

  MessageFactory factory;

  ErrMessage *message = dynamic_cast<ErrMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->message_id, 57005); // 0xdead
  EXPECT_EQ(message->display_name, "i_love_my_display_name");
  EXPECT_EQ(message->message_contents, "where are the stickers?");
}
