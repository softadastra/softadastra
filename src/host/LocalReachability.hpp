/**
 *
 *  @file LocalReachability.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_REACHABILITY_HPP
#define SOFTADASTRA_HOST_LOCAL_REACHABILITY_HPP

#include "platform/LocalDnsDelegation.hpp"
#include "platform/ManagedNetwork.hpp"

#include <cstdint>
#include <string>

namespace softadastra
{
  /**
   * @brief Describes the current local reachability state.
   */
  enum class LocalReachabilityState
  {
    Unavailable,
    Starting,
    Ready,
    Degraded
  };

  /**
   * @brief Provides the interface for the local DNS endpoint.
   */
  class LocalDnsEndpoint
  {
  public:
    /**
     * @brief Destroys the local DNS endpoint.
     */
    virtual ~LocalDnsEndpoint() = default;

    /**
     * @brief Starts the local DNS endpoint.
     *
     * @param address Address on which the endpoint should listen.
     * @param port Port on which the endpoint should listen.
     *
     * @return true if the endpoint started successfully, otherwise false.
     */
    virtual bool start(
        std::string address,
        std::uint16_t port) = 0;

    /**
     * @brief Stops the local DNS endpoint.
     */
    virtual void stop() noexcept = 0;
  };

  /**
   * @brief Provides the interface for the local gateway endpoint.
   */
  class LocalGatewayEndpoint
  {
  public:
    /**
     * @brief Destroys the local gateway endpoint.
     */
    virtual ~LocalGatewayEndpoint() = default;

    /**
     * @brief Starts the local gateway endpoint.
     *
     * @param address Address on which the endpoint should listen.
     * @param port Port on which the endpoint should listen.
     *
     * @return true if the endpoint started successfully, otherwise false.
     */
    virtual bool start(
        std::string address,
        std::uint16_t port) = 0;

    /**
     * @brief Stops the local gateway endpoint.
     */
    virtual void stop() noexcept = 0;
  };

  /**
   * @brief Coordinates managed networking, DNS, and gateway services for
   * local software reachability.
   */
  class LocalReachability
  {
  public:
    /**
     * @brief Creates a local reachability coordinator.
     *
     * @param network Managed network used for local connectivity.
     * @param delegation Local DNS delegation provider.
     * @param dns Local DNS endpoint.
     * @param gateway Local gateway endpoint.
     * @param gateway_port Port used by the local gateway.
     */
    LocalReachability(
        ManagedNetwork &network,
        const LocalDnsDelegation &delegation,
        LocalDnsEndpoint &dns,
        LocalGatewayEndpoint &gateway,
        std::uint16_t gateway_port) noexcept;

    /**
     * @brief Starts the infrastructure required for local reachability.
     *
     * @return Resulting local reachability state.
     */
    [[nodiscard]] LocalReachabilityState start();

    /**
     * @brief Stops local reachability infrastructure.
     */
    void stop() noexcept;

    /**
     * @brief Returns the current local reachability state.
     *
     * @return Current local reachability state.
     */
    [[nodiscard]] LocalReachabilityState state() const noexcept
    {
      return state_;
    }

  private:
    ManagedNetwork &network_;
    const LocalDnsDelegation &delegation_;
    LocalDnsEndpoint &dns_;
    LocalGatewayEndpoint &gateway_;
    std::uint16_t gateway_port_;
    LocalReachabilityState state_{
        LocalReachabilityState::Unavailable};
    bool dns_started_{false};
    bool gateway_started_{false};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_REACHABILITY_HPP
