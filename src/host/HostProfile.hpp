/**
 *  @file HostProfile.hpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#ifndef SOFTADASTRA_HOST_HOST_PROFILE_HPP
#define SOFTADASTRA_HOST_HOST_PROFILE_HPP

#include "platform/ManagedNetwork.hpp"
#include "host/LocalReachability.hpp"

#include <filesystem>
#include <string>

namespace softadastra
{
  enum class HostProfile { Standard, Box };
  enum class BoxState { NotProvisioned, Stopped, Ready, Degraded };

  /** Stores the explicit provisioning profile for one persistent Host identity. */
  class HostProfileStore
  {
  public:
    explicit HostProfileStore(std::filesystem::path path) noexcept;

    /** Loads the profile for host_id. A missing profile means Standard. */
    [[nodiscard]] bool load(const std::string &host_id);
    [[nodiscard]] bool provision_box(const std::string &host_id);
    [[nodiscard]] bool unprovision();
    [[nodiscard]] HostProfile profile() const noexcept;

  private:
    std::filesystem::path path_;
    HostProfile profile_{HostProfile::Standard};
  };

  [[nodiscard]] const char *host_profile_name(HostProfile profile) noexcept;
  [[nodiscard]] BoxState box_state(
      HostProfile profile,
      bool host_running,
      ManagedNetworkStatus managed_network,
      LocalReachabilityState reachability = LocalReachabilityState::Ready) noexcept;
}

#endif // SOFTADASTRA_HOST_HOST_PROFILE_HPP
