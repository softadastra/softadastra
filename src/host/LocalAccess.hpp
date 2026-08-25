/**
 *
 *  @file LocalAccess.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#ifndef SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
#define SOFTADASTRA_HOST_LOCAL_ACCESS_HPP

#include "platform/Network.hpp"
#include "platform/ManagedNetwork.hpp"
#include "host/LocalReachability.hpp"
#include "software/AccessPoint.hpp"

#include <cstdint>
#include <string>

namespace softadastra
{
  enum class LocalAccessState
  {
    Available,
    Unavailable
  };

  enum class LocalAccessNetwork
  {
    Unavailable,
    Existing,
    Managed
  };

  /**
   * @brief The current local URL resolved for a Software AccessPoint.
   *
   * The URL is derived on demand from current Host network state. It is never
   * persisted as part of the Software declaration.
   */
  struct LocalAccess
  {
    LocalAccessState state{LocalAccessState::Unavailable};
    AccessProtocol protocol{AccessProtocol::Http};
    std::uint16_t port{0};
    std::string ipv4;
    std::string url;
    LocalAccessNetwork network{LocalAccessNetwork::Unavailable};
    LocalNetworkState local_network_state{LocalNetworkState::Unavailable};
    ManagedNetworkCapability managed_network_capability{
        ManagedNetworkCapability::Unavailable};
    // Access resolution attempted the safe managed-network fallback, but it
    // did not result in a running network with a usable IPv4 address.
    bool managed_network_start_failed{false};
  };

  [[nodiscard]] LocalAccess resolve_local_access(
      AccessPoint access_point,
      NetworkCapability network_capability,
      ManagedNetworkStatus managed_network_status,
      bool software_running, std::string software_name = {},
      LocalReachabilityState reachability = LocalReachabilityState::Unavailable);

  [[nodiscard]] const char *local_access_state_name(
      LocalAccessState state) noexcept;
  [[nodiscard]] const char *local_access_network_name(
      LocalAccessNetwork network) noexcept;
}

#endif // SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
