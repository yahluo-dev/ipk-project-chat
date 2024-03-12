#include <gtest/gtest.h>
#include "../message_factory.h"
#include <string>

TEST(MessageSerializeTests, SerializeConfirm)
{
  const std::string expected = std::string("\x00\xef\xbe", 3);

  ConfirmMessage *message = new ConfirmMessage(0xbeef);

  std::string serialized = message->serialize();

  EXPECT_EQ(serialized, expected);
}
