/**
 *
 *  @file HostState.cpp
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

#include "host/HostState.hpp"
#include <algorithm>
#include <utility>

namespace softadastra
{
  bool HostState::add_software(SoftwareEntry entry)
  {
    if (find_software(entry.id()) != nullptr || (!entry.name().empty() && find_software_by_name(entry.name()) != nullptr))
    {
      return false;
    }

    software_.push_back(std::move(entry));
    return true;
  }

  bool HostState::remove_software(const SoftwareId &id)
  {
    const auto previous = software_.size();
    software_.erase(std::remove_if(software_.begin(), software_.end(), [&id](const SoftwareEntry &entry) { return entry.id() == id; }), software_.end());
    return software_.size() != previous;
  }

  SoftwareEntry *HostState::find_software(
      const SoftwareId &id) noexcept
  {
    for (auto &entry : software_)
    {
      if (entry.id() == id)
      {
        return &entry;
      }
    }

    return nullptr;
  }

  const SoftwareEntry *HostState::find_software(
      const SoftwareId &id) const noexcept
  {
    for (const auto &entry : software_)
    {
      if (entry.id() == id)
      {
        return &entry;
      }
    }

    return nullptr;
  }
  SoftwareEntry *HostState::find_software_by_name(const std::string &name) noexcept { for(auto &entry:software_) if(entry.name()==name) return &entry; return nullptr; }
  const SoftwareEntry *HostState::find_software_by_name(const std::string &name) const noexcept { for(const auto &entry:software_) if(entry.name()==name) return &entry; return nullptr; }

  SoftwareEntry *HostState::find_software(const ProjectIdentity &identity) noexcept
  {
    for (auto &entry : software_)
      if (entry.project_identity().has_value() && entry.project_identity().value() == identity)
        return &entry;
    return nullptr;
  }

  const SoftwareEntry *HostState::find_software(const ProjectIdentity &identity) const noexcept
  {
    for (const auto &entry : software_)
      if (entry.project_identity().has_value() && entry.project_identity().value() == identity)
        return &entry;
    return nullptr;
  }

  std::size_t HostState::software_count() const noexcept
  {
    return software_.size();
  }

  bool HostState::empty() const noexcept
  {
    return software_.empty();
  }

  const std::vector<SoftwareEntry> &HostState::software() const noexcept
  {
    return software_;
  }

} // namespace softadastra
