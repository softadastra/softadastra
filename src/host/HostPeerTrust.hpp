/**
 *
 *  @file HostPeerTrust.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_PEER_TRUST_HPP
#define SOFTADASTRA_HOST_HOST_PEER_TRUST_HPP

#include <string>
#include <string_view>

namespace softadastra
{
  /**
   * @brief Describes an explicitly pinned direct Host peer.
   */
  class HostPeerTrust
  {
  public:
    /**
     * @brief Creates a direct peer address and expected public HostId.
     */
    HostPeerTrust(std::string address, std::string expected_id) noexcept;

    /** @brief Returns the explicitly configured peer address. */
    [[nodiscard]] const std::string &address() const noexcept;

    /** @brief Returns the pinned public HostId. */
    [[nodiscard]] const std::string &expected_id() const noexcept;

    /** @brief Returns whether a presented peer HostId matches the pin. */
    [[nodiscard]] bool accepts(std::string_view presented_id) const noexcept;

  private:
    std::string address_;
    std::string expected_id_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_PEER_TRUST_HPP
