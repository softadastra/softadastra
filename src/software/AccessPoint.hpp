/**
 *
 *  @file AccessPoint.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#ifndef SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP
#define SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP

#include <cstdint>
#include <optional>
#include <string_view>

namespace softadastra
{
  enum class AccessProtocol
  {
    Http,
    Https
  };

  /**
   * @brief Declares one opaque network access point for registered software.
   */
  class AccessPoint
  {
  public:
    [[nodiscard]] static std::optional<AccessPoint> create(
        AccessProtocol protocol,
        std::uint16_t port) noexcept
    {
      if (port == 0)
        return std::nullopt;
      return AccessPoint(protocol, port);
    }

    [[nodiscard]] static std::optional<AccessProtocol> protocol(
        std::string_view value) noexcept
    {
      if (value == "http")
        return AccessProtocol::Http;
      if (value == "https")
        return AccessProtocol::Https;
      return std::nullopt;
    }

    [[nodiscard]] static constexpr std::string_view name(
        AccessProtocol protocol) noexcept
    {
      return protocol == AccessProtocol::Http ? "http" : "https";
    }

    [[nodiscard]] constexpr AccessProtocol protocol() const noexcept
    {
      return protocol_;
    }

    [[nodiscard]] constexpr std::uint16_t port() const noexcept
    {
      return port_;
    }

  private:
    constexpr AccessPoint(AccessProtocol protocol, std::uint16_t port) noexcept
        : protocol_(protocol), port_(port)
    {
    }

    AccessProtocol protocol_;
    std::uint16_t port_;
  };
}

#endif // SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP
