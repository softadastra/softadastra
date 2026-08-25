/**
 *  @file HostProfile.cpp
 *  Copyright 2026, Gaspard Kirira.
 *  Licensed under the Apache License, Version 2.0.
 */

#include "host/HostProfile.hpp"

#include <fstream>
#include <system_error>
#include <utility>

namespace softadastra
{
  HostProfileStore::HostProfileStore(std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool HostProfileStore::load(const std::string &host_id)
  {
    profile_ = HostProfile::Standard;
    std::error_code error;
    if (!std::filesystem::exists(path_, error))
    {
      return !error;
    }

    std::ifstream input(path_);
    std::string version;
    std::string profile;
    std::string stored_host_id;
    std::string extra;
    if (!input || !std::getline(input, version) || !std::getline(input, profile) ||
        !std::getline(input, stored_host_id) || std::getline(input, extra) ||
        version != "v1" || profile != "box" || stored_host_id != host_id ||
        host_id.empty())
    {
      return false;
    }

    profile_ = HostProfile::Box;
    return true;
  }

  bool HostProfileStore::provision_box(const std::string &host_id)
  {
    if (host_id.empty())
    {
      return false;
    }

    std::error_code error;
    std::filesystem::create_directories(path_.parent_path(), error);
    if (error)
    {
      return false;
    }

    const auto temporary = path_.string() + ".tmp";
    {
      std::ofstream output(temporary, std::ios::trunc);
      if (!output)
      {
        return false;
      }
      output << "v1\nbox\n" << host_id << '\n';
      if (!output)
      {
        return false;
      }
    }

    std::filesystem::permissions(
        temporary,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write,
        std::filesystem::perm_options::replace, error);
    if (error)
    {
      std::filesystem::remove(temporary, error);
      return false;
    }
    std::filesystem::rename(temporary, path_, error);
    if (error)
    {
      std::filesystem::remove(temporary, error);
      return false;
    }

    profile_ = HostProfile::Box;
    return true;
  }

  bool HostProfileStore::unprovision()
  {
    std::error_code error;
    std::filesystem::remove(path_, error);
    if (error)
    {
      return false;
    }
    profile_ = HostProfile::Standard;
    return true;
  }

  HostProfile HostProfileStore::profile() const noexcept
  {
    return profile_;
  }

  const char *host_profile_name(HostProfile profile) noexcept
  {
    return profile == HostProfile::Box ? "box" : "standard";
  }

  BoxState box_state(
      HostProfile profile,
      bool host_running,
      ManagedNetworkStatus managed_network,
      LocalReachabilityState reachability) noexcept
  {
    if (profile != HostProfile::Box)
    {
      return BoxState::NotProvisioned;
    }
    if (!host_running)
    {
      return BoxState::Stopped;
    }
    if (managed_network.capability != ManagedNetworkCapability::Available ||
        managed_network.state != ManagedNetworkState::Running ||
        managed_network.ipv4.empty())
    {
      return BoxState::Degraded;
    }
    return reachability == LocalReachabilityState::Ready ? BoxState::Ready : BoxState::Degraded;
  }
}
