/**
 *
 *  @file LocalAccess.cpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#include "host/LocalAccess.hpp"
#include "software/LocalName.hpp"

#include <cctype>

namespace
{
  bool usable_ipv4(const std::string &value)
  {
    unsigned int octet = 0;
    unsigned int count = 0;
    bool has_digit = false;
    bool non_zero = false;
    unsigned int first = 0;

    for (const unsigned char character : value)
    {
      if (character == '.')
      {
        if (!has_digit || octet > 255 || count == 3)
        {
          return false;
        }
        if (count == 0)
        {
          first = octet;
        }
        non_zero = non_zero || octet != 0;
        ++count;
        octet = 0;
        has_digit = false;
      }
      else if (std::isdigit(character) != 0)
      {
        has_digit = true;
        octet = octet * 10 + (character - '0');
        if (octet > 255)
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }

    if (!has_digit || count != 3 || octet > 255)
    {
      return false;
    }
    non_zero = non_zero || octet != 0;
    return non_zero && first != 127;
  }
}

namespace softadastra
{
  LocalAccess resolve_local_access(
      AccessPoint access_point,
      NetworkCapability network_capability,
      ManagedNetworkStatus managed_network_status,
      bool software_running, std::string software_name, LocalReachabilityState reachability)
  {
    LocalAccess access{
        LocalAccessState::Unavailable,
        access_point.protocol(),
        access_point.port(),
        {},
        {},
        LocalAccessNetwork::Unavailable,
        network_capability.local_network_state,
        network_capability.managed_network_capability};

    if (!software_running)
    {
      return access;
    }

    if (network_capability.state == NetworkState::Available &&
             network_capability.local_network_state == LocalNetworkState::Existing &&
             !network_capability.primary_ipv4.empty())
    {
      access.state = LocalAccessState::Available;
      access.ipv4 = std::move(network_capability.primary_ipv4);
      access.network = LocalAccessNetwork::Existing;
    }
    else if (managed_network_status.state == ManagedNetworkState::Running &&
             usable_ipv4(managed_network_status.ipv4))
    {
      access.state = LocalAccessState::Available;
      access.ipv4 = std::move(managed_network_status.ipv4);
      access.network = LocalAccessNetwork::Managed;
    }
    else
    {
      return access;
    }

    const auto local_name = LocalName::from_software_name(software_name);
    if (access.network == LocalAccessNetwork::Managed &&
        reachability == LocalReachabilityState::Ready &&
        access.protocol == AccessProtocol::Http && local_name)
      access.url = "http://" + local_name->short_name();
    else
      access.url = std::string(AccessPoint::name(access.protocol)) + "://" +
                   access.ipv4 + ":" + std::to_string(access.port);
    return access;
  }

  const char *local_access_state_name(LocalAccessState state) noexcept
  {
    return state == LocalAccessState::Available ? "available" : "unavailable";
  }

  const char *local_access_network_name(LocalAccessNetwork network) noexcept
  {
    switch (network)
    {
    case LocalAccessNetwork::Existing:
      return "existing";
    case LocalAccessNetwork::Managed:
      return "managed";
    case LocalAccessNetwork::Unavailable:
      return "unavailable";
    }

    return "unavailable";
  }
}
