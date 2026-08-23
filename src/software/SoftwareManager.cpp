/**
 *
 *  @file SoftwareManager.cpp
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

#include "software/SoftwareManager.hpp"
#include "software/SoftwareEntry.hpp"

#include <utility>

namespace softadastra
{
  SoftwareManager::SoftwareManager(HostState &state) noexcept
      : state_(state)
  {
  }

  bool SoftwareManager::register_software(SoftwareId id)
  {
    return state_.add_software(
        SoftwareEntry(std::move(id)));
  }

  bool SoftwareManager::start(
      const SoftwareId &id,
      Process &process)
  {
    auto *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return false;
    }

    entry->set_state(SoftwareState::Starting);

    if (!process.start())
    {
      entry->set_state(SoftwareState::Failed);
      return false;
    }

    entry->set_state(SoftwareState::Running);
    return true;
  }

  bool SoftwareManager::stop(
      const SoftwareId &id,
      Process &process)
  {
    auto *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return false;
    }

    if (!process.stop())
    {
      entry->set_state(SoftwareState::Failed);
      return false;
    }

    entry->set_state(SoftwareState::Stopped);
    return true;
  }

  std::optional<SoftwareState> SoftwareManager::state(
      const SoftwareId &id) const noexcept
  {
    const auto *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return std::nullopt;
    }

    return entry->state();
  }

} // namespace softadastra
