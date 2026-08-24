#include "software/AccessPoint.hpp"

#include <gtest/gtest.h>

TEST(AccessPointTest, AcceptsHttpAndHttpsPorts)
{
  const auto http = softadastra::AccessPoint::protocol("http");
  const auto https = softadastra::AccessPoint::protocol("https");

  ASSERT_TRUE(http.has_value());
  ASSERT_TRUE(https.has_value());
  EXPECT_TRUE(softadastra::AccessPoint::create(http.value(), 8080).has_value());
  EXPECT_TRUE(softadastra::AccessPoint::create(https.value(), 8443).has_value());
}

TEST(AccessPointTest, RejectsUnknownProtocolAndInvalidPorts)
{
  const auto http = softadastra::AccessPoint::protocol("http");
  ASSERT_TRUE(http.has_value());
  EXPECT_FALSE(softadastra::AccessPoint::protocol("tcp").has_value());
  EXPECT_FALSE(softadastra::AccessPoint::create(http.value(), 0).has_value());
}
