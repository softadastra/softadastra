/**
 *
 *  @file HostPeerIdentity.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_PEER_IDENTITY_HPP
#define SOFTADASTRA_HOST_HOST_PEER_IDENTITY_HPP

#include <string_view>

namespace softadastra
{
  /**
   * @brief Validates and compares public persistent Host identities.
   *
   * This type never handles the Host administration secret.
   */
  class HostPeerIdentity
  {
  public:
    /**
     * @brief Returns whether an identity has the canonical public format.
     */
    [[nodiscard]] static bool valid(std::string_view identity) noexcept;

    /**
     * @brief Compares a presented identity with an explicitly expected Host.
     */
    [[nodiscard]] static bool matches(
        std::string_view expected,
        std::string_view presented) noexcept;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_PEER_IDENTITY_HPP
