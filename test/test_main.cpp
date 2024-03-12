#include <gtest/gtest.h>
#include "../message_factory.h"
#include <string>

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

TEST(MsgFactoryTests, CreateAuth)
{
  std::string binary_message="\x02\xad\xde";
  binary_message += std::string("buttercat3323\x00", 14); 
  binary_message += std::string("buttercat1234\x00", 14);
  binary_message += std::string("topsecret\x00", 10);

  MessageFactory factory;

  AuthMessage *message = dynamic_cast<AuthMessage *>(factory.create(binary_message));

  EXPECT_EQ(message->username, "buttercat3323");
  EXPECT_EQ(message->secret, "topsecret");
  EXPECT_EQ(message->displayname, "buttercat1234");
  EXPECT_EQ(message->message_id, 57005); // 0xdead
}
