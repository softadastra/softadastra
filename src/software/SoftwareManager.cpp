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

#include <algorithm>
#include <utility>

namespace softadastra
{
  SoftwareManager::SoftwareManager(
      HostState &state,
      ProcessLauncher &process_launcher) noexcept
      : state_(state),
        process_launcher_(process_launcher)
  {
  }

  bool SoftwareManager::register_software(
      SoftwareId id,
      ProcessSpec process_spec)
  {
    return state_.add_software(
        SoftwareEntry(
            std::move(id),
            std::move(process_spec)));
  }

  bool SoftwareManager::start(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return false;
    }

    ManagedProcess *existing = find_process(id);

    if (existing != nullptr && existing->process->is_running())
    {
      return false;
    }

    if (existing != nullptr)
    {
      processes_.erase(
          std::remove_if(
              processes_.begin(),
              processes_.end(),
              [&id](const ManagedProcess &managed)
              {
                return managed.id == id;
              }),
          processes_.end());
    }

    entry->set_state(SoftwareState::Starting);

    std::unique_ptr<Process> process =
        process_launcher_.launch(entry->process_spec());

    if (process == nullptr)
    {
      entry->set_state(SoftwareState::Failed);
      return false;
    }

    if (!process->is_running())
    {
      entry->set_state(SoftwareState::Failed);
      return false;
    }

    processes_.push_back(
        ManagedProcess{
            entry->id(),
            std::move(process)});

    entry->set_state(SoftwareState::Running);

    return true;
  }

  bool SoftwareManager::stop(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return false;
    }

    ManagedProcess *managed = find_process(id);

    if (managed == nullptr)
    {
      return false;
    }

    if (!managed->process->stop())
    {
      entry->set_state(SoftwareState::Failed);
      return false;
    }

    processes_.erase(
        std::remove_if(
            processes_.begin(),
            processes_.end(),
            [&id](const ManagedProcess &current)
            {
              return current.id == id;
            }),
        processes_.end());

    entry->set_state(SoftwareState::Stopped);

    return true;
  }

  std::optional<SoftwareState> SoftwareManager::state(
      const SoftwareId &id) const noexcept
  {
    const SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return std::nullopt;
    }

    return entry->state();
  }

  SoftwareManager::ManagedProcess *SoftwareManager::find_process(
      const SoftwareId &id) noexcept
  {
    const auto it = std::find_if(
        processes_.begin(),
        processes_.end(),
        [&id](const ManagedProcess &managed)
        {
          return managed.id == id;
        });

    if (it == processes_.end())
    {
      return nullptr;
    }

    return &(*it);
  }

  const SoftwareManager::ManagedProcess *SoftwareManager::find_process(
      const SoftwareId &id) const noexcept
  {
    const auto it = std::find_if(
        processes_.begin(),
        processes_.end(),
        [&id](const ManagedProcess &managed)
        {
          return managed.id == id;
        });

    if (it == processes_.end())
    {
      return nullptr;
    }

    return &(*it);
  }

} // namespace softadastra
