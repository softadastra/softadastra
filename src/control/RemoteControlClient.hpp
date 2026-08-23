/**
 *
 *  @file RemoteControlClient.hpp
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

#ifndef SOFTADASTRA_CONTROL_REMOTE_CONTROL_CLIENT_HPP
#define SOFTADASTRA_CONTROL_REMOTE_CONTROL_CLIENT_HPP

#include <cstdint>
#include <optional>
#include <string>

namespace softadastra
{
  /** @brief Sends one authenticated control request over TLS 1.3. */
  class RemoteControlClient
  {
  public:
    /** @brief Creates a client for an explicitly configured Host endpoint. */
    RemoteControlClient(std::string address, std::uint16_t port, std::string secret) noexcept;

    /** @brief Sends an authenticated Host control-protocol command. */
    [[nodiscard]] std::optional<std::string> request(const std::string &command) const noexcept;

  private:
    std::string address_;
    std::uint16_t port_;
    std::string secret_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_REMOTE_CONTROL_CLIENT_HPP
