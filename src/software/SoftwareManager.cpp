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

    if (existing != nullptr)
    {
      if (existing->process->is_running())
      {
        return false;
      }

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
      const auto code = process->exit_code();

      entry->set_state(
          code.has_value() && code.value() == 0
              ? SoftwareState::Stopped
              : SoftwareState::Failed);

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

  bool SoftwareManager::restart(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return false;
    }

    ManagedProcess *managed = find_process(id);

    if (managed != nullptr)
    {
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
    }

    return start(id);
  }

  void SoftwareManager::refresh()
  {
    auto current = processes_.begin();

    while (current != processes_.end())
    {
      if (current->process->is_running())
      {
        ++current;
        continue;
      }

      SoftwareEntry *entry =
          state_.find_software(current->id);

      if (entry != nullptr)
      {
        const auto code =
            current->process->exit_code();

        if (code.has_value() && code.value() == 0)
        {
          entry->set_state(SoftwareState::Stopped);
        }
        else
        {
          entry->set_state(SoftwareState::Failed);
        }
      }

      current = processes_.erase(current);
    }
  }

  std::optional<SoftwareState> SoftwareManager::state(
      const SoftwareId &id) const noexcept
  {
    const SoftwareEntry *entry =
        state_.find_software(id);

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
