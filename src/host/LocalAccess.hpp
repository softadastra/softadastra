/**
 *
 *  @file LocalAccess.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#ifndef SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
#define SOFTADASTRA_HOST_LOCAL_ACCESS_HPP

#include "platform/Network.hpp"
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
    LocalNetworkState local_network_state{LocalNetworkState::Unavailable};
    ManagedNetworkCapability managed_network_capability{
        ManagedNetworkCapability::Unavailable};
  };

  [[nodiscard]] LocalAccess resolve_local_access(
      AccessPoint access_point,
      NetworkCapability network_capability,
      bool software_running);

  [[nodiscard]] const char *local_access_state_name(
      LocalAccessState state) noexcept;
}

#endif // SOFTADASTRA_HOST_LOCAL_ACCESS_HPP
