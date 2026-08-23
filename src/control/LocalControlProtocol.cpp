/**
 *
 *  @file LocalControlProtocol.cpp
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

#include "control/LocalControlProtocol.hpp"

#include <charconv>
#include <cctype>

namespace softadastra
{
  namespace
  {
    constexpr char digits[] = "0123456789abcdef";

    int hex_value(char value) noexcept
    {
      if (value >= '0' && value <= '9')
      {
        return value - '0';
      }

      if (value >= 'a' && value <= 'f')
      {
        return value - 'a' + 10;
      }

      return -1;
    }
  } // namespace

  std::string LocalControlProtocol::encode(std::string_view value)
  {
    if (value.empty())
    {
      return "-";
    }

    std::string encoded;
    encoded.reserve(value.size() * 2);

    for (const unsigned char character : value)
    {
      encoded += digits[character >> 4U];
      encoded += digits[character & 0x0fU];
    }

    return encoded;
  }

  std::optional<std::string> LocalControlProtocol::decode(
      std::string_view value)
  {
    if (value == "-")
    {
      return std::string{};
    }

    if (value.size() % 2 != 0)
    {
      return std::nullopt;
    }

    std::string decoded;
    decoded.reserve(value.size() / 2);

    for (std::size_t index = 0; index < value.size(); index += 2)
    {
      const int high = hex_value(value[index]);
      const int low = hex_value(value[index + 1]);

      if (high < 0 || low < 0)
      {
        return std::nullopt;
      }

      decoded += static_cast<char>((high << 4) | low);
    }

    return decoded;
  }

  std::vector<std::string> LocalControlProtocol::fields(
      std::string_view message)
  {
    std::vector<std::string> result;
    std::size_t offset = 0;

    while (offset < message.size())
    {
      while (offset < message.size() &&
             std::isspace(static_cast<unsigned char>(message[offset])) != 0)
      {
        ++offset;
      }

      const std::size_t begin = offset;

      while (offset < message.size() &&
             std::isspace(static_cast<unsigned char>(message[offset])) == 0)
      {
        ++offset;
      }

      if (begin != offset)
      {
        result.emplace_back(message.substr(begin, offset - begin));
      }
    }

    return result;
  }

  std::optional<int> LocalControlProtocol::integer(std::string_view value)
  {
    int result = 0;
    const auto parsed = std::from_chars(
        value.data(),
        value.data() + value.size(),
        result);

    if (parsed.ec != std::errc() ||
        parsed.ptr != value.data() + value.size())
    {
      return std::nullopt;
    }

    return result;
  }

} // namespace softadastra
