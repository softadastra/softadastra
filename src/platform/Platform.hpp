/**
 *
 *  @file Platform.hpp
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

#ifndef SOFTADASTRA_PLATFORM_PLATFORM_HPP
#define SOFTADASTRA_PLATFORM_PLATFORM_HPP

#include "platform/LocalFirewall.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/Network.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/Service.hpp"

namespace softadastra
{
  /**
   * @brief Provides access to platform-specific infrastructure capabilities.
   *
   * Platform exposes the operating-system capabilities required by the Host
   * while keeping Host policy and state independent from a specific platform
   * implementation.
   */
  class Platform
  {
  public:
    /**
     * @brief Destroys the platform provider.
     */
    virtual ~Platform() = default;

    /**
     * @brief Returns the process launching capability.
     *
     * @return Mutable process launcher.
     */
    [[nodiscard]] virtual ProcessLauncher &process_launcher() noexcept = 0;

    /**
     * @brief Returns the process launching capability.
     *
     * @return Read-only process launcher.
     */
    [[nodiscard]] virtual const ProcessLauncher &process_launcher()
        const noexcept = 0;

    /**
     * @brief Returns the system service capability.
     *
     * @return Mutable service provider.
     */
    [[nodiscard]] virtual Service &service() noexcept = 0;

    /**
     * @brief Returns the system service capability.
     *
     * @return Read-only service provider.
     */
    [[nodiscard]] virtual const Service &service() const noexcept = 0;

    /**
     * @brief Returns the network capability.
     *
     * @return Mutable network provider.
     */
    [[nodiscard]] virtual Network &network() noexcept = 0;

    /**
     * @brief Returns the network capability.
     *
     * @return Read-only network provider.
     */
    [[nodiscard]] virtual const Network &network() const noexcept = 0;

    /**
     * @brief Returns the local firewall capability.
     *
     * Platforms that do not provide a local firewall use the unavailable
     * implementation.
     *
     * @return Mutable local firewall provider.
     */
    [[nodiscard]] virtual LocalFirewall &local_firewall() noexcept
    {
      static UnavailableLocalFirewall firewall;

      return firewall;
    }

    /**
     * @brief Returns the local firewall capability.
     *
     * Platforms that do not provide a local firewall use the unavailable
     * implementation.
     *
     * @return Read-only local firewall provider.
     */
    [[nodiscard]] virtual const LocalFirewall &local_firewall() const noexcept
    {
      static UnavailableLocalFirewall firewall;

      return firewall;
    }

    /**
     * @brief Returns the managed network capability.
     *
     * Platforms that do not provide managed networking use the unavailable
     * implementation.
     *
     * @return Mutable managed network provider.
     */
    [[nodiscard]] virtual ManagedNetwork &managed_network() noexcept
    {
      static UnavailableManagedNetwork network;

      return network;
    }

    /**
     * @brief Returns the managed network capability.
     *
     * Platforms that do not provide managed networking use the unavailable
     * implementation.
     *
     * @return Read-only managed network provider.
     */
    [[nodiscard]] virtual const ManagedNetwork &managed_network() const noexcept
    {
      static UnavailableManagedNetwork network;

      return network;
    }
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PLATFORM_HPP
