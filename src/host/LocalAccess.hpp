/**
 *
 *  @file LocalAccess.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
#define SOFTADASTRA_HOST_LOCAL_ACCESS_HPP

#include "host/LocalReachability.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/Network.hpp"
#include "software/AccessPoint.hpp"

#include <cstdint>
#include <string>

namespace softadastra
{
  /**
   * @brief Describes whether local access to software is available.
   */
  enum class LocalAccessState
  {
    Available,
    Unavailable
  };

  /**
   * @brief Identifies the network used for local software access.
   */
  enum class LocalAccessNetwork
  {
    Unavailable,
    Existing,
    Managed
  };

  /** Reports whether local firewall policy has made the declared port usable. */
  enum class LocalAccessFirewallState
  {
    NotRequired,
    Open,
    PermissionRequired,
    Unsupported,
    Failed
  };

  /**
   * @brief Describes the current local access resolved for a software AccessPoint.
   *
   * Local access is derived from the current Host network state and is not
   * persisted as part of the software declaration.
   */
  struct LocalAccess
  {
    /**
     * @brief Current local access state.
     */
    LocalAccessState state{
        LocalAccessState::Unavailable};

    /**
     * @brief Protocol exposed by the software AccessPoint.
     */
    AccessProtocol protocol{
        AccessProtocol::Http};

    /**
     * @brief Port exposed by the software AccessPoint.
     */
    std::uint16_t port{0};

    /**
     * @brief IPv4 address used for local access.
     */
    std::string ipv4;

    /**
     * @brief Resolved local access URL.
     */
    std::string url;

    /**
     * @brief Network selected for local access.
     */
    LocalAccessNetwork network{
        LocalAccessNetwork::Unavailable};

    /**
     * @brief State of the existing local network.
     */
    LocalNetworkState local_network_state{
        LocalNetworkState::Unavailable};

    /**
     * @brief Managed network capability reported by the Host.
     */
    ManagedNetworkCapability managed_network_capability{
        ManagedNetworkCapability::Unavailable};

    /**
     * @brief Indicates that the managed-network fallback did not provide
     * usable local access.
     */
    bool managed_network_start_failed{false};

    /** Firewall policy result for an existing local network. */
    LocalAccessFirewallState firewall{
        LocalAccessFirewallState::NotRequired};

    /** Local IPv4 subnet to which any Host-owned rule is restricted. */
    std::string local_subnet;
  };

  /**
   * @brief Resolves local access for a software AccessPoint.
   *
   * @param access_point AccessPoint exposed by the software.
   * @param network_capability Current Host network capability.
   * @param managed_network_status Current managed network status.
   * @param software_running Whether the software is currently running.
   * @param software_name Software name used for local name resolution.
   * @param reachability Current local reachability state.
   *
   * @return Resolved local access information.
   */
  [[nodiscard]] LocalAccess resolve_local_access(
      AccessPoint access_point,
      NetworkCapability network_capability,
      ManagedNetworkStatus managed_network_status,
      bool software_running,
      std::string software_name = {},
      LocalReachabilityState reachability =
          LocalReachabilityState::Unavailable);

  /**
   * @brief Returns the canonical name of a local access state.
   *
   * @param state Local access state.
   *
   * @return Canonical state name.
   */
  [[nodiscard]] const char *local_access_state_name(
      LocalAccessState state) noexcept;

  /**
   * @brief Returns the canonical name of a local access network.
   *
   * @param network Local access network.
   *
   * @return Canonical network name.
   */
  [[nodiscard]] const char *local_access_network_name(
      LocalAccessNetwork network) noexcept;

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
