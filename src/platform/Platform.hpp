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

#include "platform/Network.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/LocalFirewall.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/Service.hpp"

namespace softadastra
{
  /**
   * @brief Provides access to platform-specific infrastructure capabilities.
   *
   * Platform defines process, service, and network capabilities used by the
   * Host. It owns neither process lifecycle policy nor Host state, keeping the
   * Host model independent from a particular operating system.
   */
  class Platform
  {
  public:
    virtual ~Platform() = default;

    /**
     * @brief Returns the process launching capability.
     */
    [[nodiscard]] virtual ProcessLauncher &process_launcher() noexcept = 0;

    /**
     * @brief Returns the process launching capability.
     */
    [[nodiscard]] virtual const ProcessLauncher &process_launcher()
        const noexcept = 0;

    /**
     * @brief Returns the system service capability.
     */
    [[nodiscard]] virtual Service &service() noexcept = 0;

    /**
     * @brief Returns the system service capability.
     */
    [[nodiscard]] virtual const Service &service() const noexcept = 0;

    /**
     * @brief Returns the network capability.
     */
    [[nodiscard]] virtual Network &network() noexcept = 0;

    /**
     * @brief Returns the network capability.
     */
    [[nodiscard]] virtual const Network &network() const noexcept = 0;

    /** Returns the local firewall capability, when the platform supports it. */
    [[nodiscard]] virtual LocalFirewall &local_firewall() noexcept
    { static UnavailableLocalFirewall firewall; return firewall; }
    [[nodiscard]] virtual const LocalFirewall &local_firewall() const noexcept
    { static UnavailableLocalFirewall firewall; return firewall; }

    [[nodiscard]] virtual ManagedNetwork &managed_network() noexcept
    { static UnavailableManagedNetwork network; return network; }
    [[nodiscard]] virtual const ManagedNetwork &managed_network() const noexcept
    { static UnavailableManagedNetwork network; return network; }
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_PLATFORM_HPP
