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

#include <algorithm>
#include <array>
#include <cstdint>
#include <string_view>
#include <utility>
#include <vector>

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
  EXPECT_EQ(encoded.size(), 25);
  EXPECT_FALSE(qr.empty());
  EXPECT_EQ(qr.find("\x1b"), std::string::npos);
  EXPECT_NE(qr.find("▀"), std::string::npos);
  EXPECT_NE(qr.find("▄"), std::string::npos);
  EXPECT_NE(qr.find("█"), std::string::npos);
}

TEST(AccessUrlTest, EncodesKnownUrlAsReferenceCodewords)
{
  constexpr std::string_view url = "http://192.168.1.6:8080";
  constexpr std::array<std::uint8_t, 44> expected = {
      65, 118, 135, 71, 71, 3, 162, 242, 243, 19, 147, 34, 227, 19, 99,
      130, 227, 18, 227, 99, 163, 131, 3, 131, 0, 236, 17, 236, 196, 202,
      242, 167, 180, 36, 142, 160, 149, 253, 52, 109, 57, 137, 181, 178,
  };

  const auto encoded = softadastra::internal::generate(url);

  ASSERT_EQ(encoded.size(), 25);

  std::uint16_t format = 0;
  for (int bit = 0; bit < 15; ++bit)
  {
    const auto [row, col] = bit < 6 ? std::pair{8, bit}
                          : bit == 6 ? std::pair{8, 7}
                          : bit == 7 ? std::pair{8, 8}
                          : bit == 8 ? std::pair{7, 8}
                                     : std::pair{14 - bit, 8};
    format |= static_cast<std::uint16_t>(encoded.module(row, col)) << bit;
  }

  int mask = -1;
  for (int candidate = 0; candidate < 8; ++candidate)
    if (format == softadastra::internal::FORMAT_INFO[1][candidate])
      mask = candidate;

  ASSERT_GE(mask, 0);

  const auto is_masked = [mask](int row, int col)
  {
    switch (mask)
    {
    case 0: return (row + col) % 2 == 0;
    case 1: return row % 2 == 0;
    case 2: return col % 3 == 0;
    case 3: return (row + col) % 3 == 0;
    case 4: return (row / 2 + col / 3) % 2 == 0;
    case 5: return (row * col) % 2 + (row * col) % 3 == 0;
    case 6: return ((row * col) % 2 + (row * col) % 3) % 2 == 0;
    case 7: return ((row + col) % 2 + (row * col) % 3) % 2 == 0;
    default: return false;
    }
  };

  std::vector<std::uint8_t> actual;
  std::uint8_t byte = 0;
  int bit_count = 0;
  bool upward = true;
  int column = encoded.size() - 1;
  while (column >= 1)
  {
    if (column == 6)
      --column;

    for (int offset = 0; offset < encoded.size(); ++offset)
    {
      const int row = upward ? encoded.size() - 1 - offset : offset;
      for (int delta = 0; delta < 2; ++delta)
      {
        const int col = column - delta;
        if (encoded.is_function_module(row, col))
          continue;

        const bool value = encoded.module(row, col) ^ is_masked(row, col);
        byte = static_cast<std::uint8_t>((byte << 1U) | value);
        if (++bit_count == 8)
        {
          actual.push_back(byte);
          byte = 0;
          bit_count = 0;
        }
      }
    }
    upward = !upward;
    column -= 2;
  }

  ASSERT_EQ(actual.size(), expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i)
    EXPECT_EQ(actual[i], expected[i]) << "codeword=" << i;
}

TEST(AccessUrlTest, UsesCompactAnsiTerminalRendererWithQuietZone)
{
  const auto encoded = softadastra::internal::generate("http://192.168.1.6:8080");
  const std::string terminal = encoded.to_ascii({.use_ansi = true});

  EXPECT_EQ(std::count(terminal.begin(), terminal.end(), '\n'), 17);

  EXPECT_NE(terminal.find("\u2584"), std::string::npos);
  EXPECT_EQ(terminal.find("\u2580"), std::string::npos);
  EXPECT_EQ(terminal.find("\u2588"), std::string::npos);

  EXPECT_NE(terminal.find("\x1b[30;40m"), std::string::npos);
  EXPECT_NE(terminal.find("\x1b[37;47m"), std::string::npos);
  EXPECT_NE(terminal.find("\x1b[37;40m"), std::string::npos);
  EXPECT_NE(terminal.find("\x1b[0m\n"), std::string::npos);

  const std::string tail = "\x1b[0m\n";
  EXPECT_EQ(terminal.compare(terminal.size() - tail.size(), tail.size(), tail), 0);
}
