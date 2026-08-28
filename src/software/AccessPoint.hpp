/**
 *
 *  @file AccessPoint.hpp
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

#ifndef SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP
#define SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP

#include <cstdint>
#include <optional>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Identifies the network protocol exposed by an access point.
   */
  enum class AccessProtocol
  {
    Http,
    Https,
    Ws
  };

  /**
   * @brief Declares one opaque network access point for registered software.
   *
   * An access point associates a supported network protocol with the port
   * through which registered software can be reached.
   */
  class AccessPoint
  {
  public:
    /**
     * @brief Creates an access point.
     *
     * @param protocol Network protocol exposed by the software.
     * @param port Port on which the software is reachable.
     *
     * @return An access point when the port is valid, or std::nullopt when
     *         the port is zero.
     */
    [[nodiscard]] static std::optional<AccessPoint> create(
        AccessProtocol protocol,
        std::uint16_t port) noexcept
    {
      if (port == 0)
      {
        return std::nullopt;
      }

      return AccessPoint(protocol, port);
    }

    /**
     * @brief Parses an access protocol name.
     *
     * @param value Protocol name to parse.
     *
     * @return The corresponding access protocol, or std::nullopt if the
     *         value is not recognized.
     */
    [[nodiscard]] static std::optional<AccessProtocol> protocol(
        std::string_view value) noexcept
    {
      if (value == "http")
      {
        return AccessProtocol::Http;
      }

      if (value == "https")
      {
        return AccessProtocol::Https;
      }

      if (value == "ws")
      {
        return AccessProtocol::Ws;
      }

      return std::nullopt;
    }

    /**
     * @brief Returns the canonical name of an access protocol.
     *
     * @param protocol Access protocol to convert.
     *
     * @return Canonical protocol name.
     */
    [[nodiscard]] static constexpr std::string_view name(
        AccessProtocol protocol) noexcept
    {
      return protocol == AccessProtocol::Http
                 ? "http"
             : protocol == AccessProtocol::Https
                 ? "https"
                 : "ws";
    }

    /**
     * @brief Returns the access protocol.
     *
     * @return Protocol exposed by the access point.
     */
    [[nodiscard]] constexpr AccessProtocol protocol() const noexcept
    {
      return protocol_;
    }

    /**
     * @brief Returns the access port.
     *
     * @return Port exposed by the access point.
     */
    [[nodiscard]] constexpr std::uint16_t port() const noexcept
    {
      return port_;
    }

    [[nodiscard]] friend constexpr bool operator==(
        AccessPoint,
        AccessPoint) noexcept = default;

  private:
    constexpr AccessPoint(
        AccessProtocol protocol,
        std::uint16_t port) noexcept
        : protocol_(protocol),
          port_(port)
    {
    }

    AccessProtocol protocol_;
    std::uint16_t port_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_SOFTWARE_ACCESS_POINT_HPP
