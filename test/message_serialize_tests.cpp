#include <gtest/gtest.h>
#include "../message_factory.h"
#include <string>

TEST(MessageSerializeTests, SerializeConfirm)
{
  const std::string expected = std::string("\x00\xbe\xef", 3);

  auto *message = new ConfirmMessage(0xbeef);

  std::string serialized = message->serialize();

  EXPECT_EQ(serialized, expected);
}

TEST(MessageSerializeTests, SerializeAuth)
{
  std::string expected = std::string("\x02\xbe\xef");
  expected += std::string("user123\x00", 8);
  expected += std::string("display_name\x00", 13);
  expected += std::string("user_secret\x00", 12);

  auto *message = new AuthMessage("user123", "user_secret", "display_name",  0xbeef);

  std::string serialized = message->serialize();

  EXPECT_EQ(serialized, expected);
}

TEST(MessageSerializeTests, SerializeMsg)
{
  std::string expected = std::string("\x04\xbe\xef");
  expected += std::string("display_name\x00", 13);
  expected += std::string("message_contents\x00", 17);

  auto *message = new MsgMessage(0xbeef, "display_name", "message_contents");

  std::string serialized = message->serialize();

  EXPECT_EQ(serialized, expected);
}
