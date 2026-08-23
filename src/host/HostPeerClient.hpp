/**
 *
 *  @file HostPeerClient.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_PEER_CLIENT_HPP
#define SOFTADASTRA_HOST_HOST_PEER_CLIENT_HPP

#include "host/HostPeerTrust.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace softadastra
{
  /** @brief Connects to an explicitly pinned TLS Host peer. */
  class HostPeerClient
  {
  public:
    /** @brief Creates a client for one configured peer and TCP port. */
    HostPeerClient(HostPeerTrust trust, std::uint16_t port) noexcept;

    /** @brief Requests identity, ping or non-sensitive infrastructure info. */
    [[nodiscard]] std::optional<std::string> request(
        std::string_view command) const noexcept;

  private:
    HostPeerTrust trust_;
    std::uint16_t port_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_PEER_CLIENT_HPP
