/**
 *
 *  @file ManagedNetwork.hpp
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

#ifndef SOFTADASTRA_PLATFORM_MANAGED_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_MANAGED_NETWORK_HPP

#include "platform/Network.hpp"

#include <string>

namespace softadastra
{
  /**
   * @brief Describes the runtime state of a managed network.
   */
  enum class ManagedNetworkState
  {
    Stopped,
    Running
  };

  /**
   * @brief Describes the result of starting a managed network.
   */
  enum class ManagedNetworkStartResult
  {
    Started,
    AlreadyRunning,
    Unavailable,
    WouldDisruptConnection,
    Failed
  };

  /**
   * @brief Describes the current state and addressing of a managed network.
   */
  struct ManagedNetworkStatus
  {
    /**
     * @brief Indicates whether managed networking is available.
     */
    ManagedNetworkCapability capability{
        ManagedNetworkCapability::Unavailable};

    /**
     * @brief Current runtime state of the managed network.
     */
    ManagedNetworkState state{
        ManagedNetworkState::Stopped};

    /**
     * @brief Name of the network interface used by the managed network.
     */
    std::string interface_name;

    /**
     * @brief IPv4 address assigned to the managed network interface.
     */
    std::string ipv4;

    /**
     * @brief Wireless network name associated with the managed network.
     */
    std::string ssid;
  };

  /**
   * @brief Provides the platform interface for managing a local network.
   */
  class ManagedNetwork
  {
  public:
    /**
     * @brief Destroys the managed network interface.
     */
    virtual ~ManagedNetwork() = default;

    /**
     * @brief Returns the current managed network status.
     *
     * @return Current managed network status.
     */
    [[nodiscard]] virtual ManagedNetworkStatus status() const = 0;

    /**
     * @brief Starts the managed network.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] virtual ManagedNetworkStartResult start() = 0;

    /**
     * @brief Stops the managed network.
     *
     * @return true if the managed network was stopped successfully,
     *         otherwise false.
     */
    virtual bool stop() = 0;
  };

  /**
   * @brief Represents a platform without managed network support.
   */
  class UnavailableManagedNetwork final : public ManagedNetwork
  {
  public:
    /**
     * @brief Returns the unavailable managed network status.
     *
     * @return Default unavailable managed network status.
     */
    [[nodiscard]] ManagedNetworkStatus status() const override
    {
      return {};
    }

    /**
     * @brief Reports that managed networking is unavailable.
     *
     * @return ManagedNetworkStartResult::Unavailable.
     */
    [[nodiscard]] ManagedNetworkStartResult start() override
    {
      return ManagedNetworkStartResult::Unavailable;
    }

    /**
     * @brief Reports that the managed network cannot be stopped.
     *
     * @return false.
     */
    bool stop() override
    {
      return false;
    }
  };

  /**
   * @brief Returns the canonical name of a managed network state.
   *
   * @param state Managed network state.
   *
   * @return Canonical state name.
   */
  [[nodiscard]] const char *managed_network_state_name(
      ManagedNetworkState state) noexcept;

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_MANAGED_NETWORK_HPP
