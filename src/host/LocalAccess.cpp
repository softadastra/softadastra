/**
 *
 *  @file LocalAccess.cpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#include "host/LocalAccess.hpp"

namespace softadastra
{
  LocalAccess resolve_local_access(
      AccessPoint access_point,
      NetworkCapability network_capability,
      bool software_running)
  {
    LocalAccess access{
        LocalAccessState::Unavailable,
        access_point.protocol(),
        access_point.port(),
        {},
        {},
        network_capability.local_network_state,
        network_capability.managed_network_capability};

    if (!software_running ||
        network_capability.state != NetworkState::Available ||
        network_capability.local_network_state != LocalNetworkState::Existing ||
        network_capability.primary_ipv4.empty())
    {
      return access;
    }

    access.state = LocalAccessState::Available;
    access.ipv4 = std::move(network_capability.primary_ipv4);
    access.url = std::string(AccessPoint::name(access.protocol)) + "://" +
                 access.ipv4 + ":" + std::to_string(access.port);
    return access;
  }

  const char *local_access_state_name(LocalAccessState state) noexcept
  {
    return state == LocalAccessState::Available ? "available" : "unavailable";
  }
}
