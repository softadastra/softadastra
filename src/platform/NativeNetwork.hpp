/**
 *
 *  @file NativeNetwork.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_NETWORK_HPP

#include "platform/Network.hpp"

namespace softadastra
{
  /**
   * @brief Provides network state from the native operating system.
   *
   * NativeNetwork reports whether the machine has a usable non-loopback
   * network interface and whether at least one such interface is currently
   * active with a network address.
   *
   * These operations describe local network state. They do not imply Internet
   * reachability.
   */
  class NativeNetwork final : public Network
  {
  public:
    /**
     * @brief Returns whether a usable network interface is available.
     *
     * @return true when the machine exposes at least one non-loopback network
     *         interface, otherwise false.
     */
    [[nodiscard]] bool is_available() const noexcept override;

    /**
     * @brief Returns whether a usable network interface is connected.
     *
     * @return true when at least one non-loopback interface is active and has
     *         an IPv4 or IPv6 address, otherwise false.
     */
    [[nodiscard]] bool is_connected() const noexcept override;

    /**
     * @brief Returns the native hostname when the operating system provides it.
     */
    [[nodiscard]] std::string host_name() const override;

    /**
     * @brief Returns active non-loopback IPv4 and IPv6 addresses.
     */
    [[nodiscard]] std::vector<LocalNetworkAddress> local_addresses() const override;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_NETWORK_HPP
