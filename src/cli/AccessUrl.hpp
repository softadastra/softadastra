/**
 *
 *  @file AccessUrl.hpp
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

#ifndef SOFTADASTRA_CLI_ACCESS_URL_HPP
#define SOFTADASTRA_CLI_ACCESS_URL_HPP

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Builds local IPv4 HTTP access URLs.
   */
  class AccessUrl
  {
  public:
    /**
     * @brief Parses a valid TCP port number.
     */
    [[nodiscard]] static std::optional<std::uint16_t> port(
        std::string_view value) noexcept;

    /**
     * @brief Builds an HTTP URL for an IPv4 address and port.
     */
    [[nodiscard]] static std::string http(
        std::string_view ipv4,
        std::uint16_t port);
  };

} // namespace softadastra

#endif // SOFTADASTRA_CLI_ACCESS_URL_HPP
