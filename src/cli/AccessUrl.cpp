/**
 *
 *  @file AccessUrl.cpp
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

#include <charconv>

namespace softadastra
{
  std::optional<std::uint16_t> AccessUrl::port(
      std::string_view value) noexcept
  {
    unsigned int port = 0;
    const auto result = std::from_chars(
        value.data(), value.data() + value.size(), port);

    if (result.ec != std::errc() || result.ptr != value.data() + value.size() ||
        port == 0 || port > 65535)
    {
      return std::nullopt;
    }

    return static_cast<std::uint16_t>(port);
  }

  std::string AccessUrl::http(
      std::string_view ipv4,
      std::uint16_t port)
  {
    return "http://" + std::string(ipv4) + ":" + std::to_string(port);
  }

  std::string AccessUrl::https(std::string_view ipv4, std::uint16_t port)
  {
    return "https://" + std::string(ipv4) + ":" + std::to_string(port);
  }

} // namespace softadastra
