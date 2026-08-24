/**
 *
 *  @file access_url_test.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#include "cli/AccessUrl.hpp"
#include "internal/QrEncoder.hpp"
#include "platform/QrCode.hpp"

#include <gtest/gtest.h>

TEST(AccessUrlTest, BuildsUrlFromCurrentIpv4AndPort)
{
  EXPECT_EQ(
      softadastra::AccessUrl::http("192.168.1.6", 8080),
      "http://192.168.1.6:8080");
}

TEST(AccessUrlTest, AcceptsBoundaryPorts)
{
  EXPECT_EQ(softadastra::AccessUrl::port("1"), 1);
  EXPECT_EQ(softadastra::AccessUrl::port("65535"), 65535);
}

TEST(AccessUrlTest, RejectsInvalidPorts)
{
  EXPECT_FALSE(softadastra::AccessUrl::port("0").has_value());
  EXPECT_FALSE(softadastra::AccessUrl::port("65536").has_value());
}

TEST(AccessUrlTest, RendersExpectedUrlWithoutExternalGenerator)
{
  const std::string url = softadastra::AccessUrl::http("192.168.1.6", 8080);
  const auto encoded = softadastra::internal::generate(url);
  const std::string qr = softadastra::QrCode::render(url);

  EXPECT_EQ(url, "http://192.168.1.6:8080");
  EXPECT_EQ(encoded.original_data(), url);
  EXPECT_FALSE(encoded.to_ascii().empty());
  EXPECT_FALSE(qr.empty());
  EXPECT_NE(qr.find("██"), std::string::npos);
}
