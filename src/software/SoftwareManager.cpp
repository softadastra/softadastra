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
  namespace
  {
    SoftwareOperationError software_error(
        ProcessLaunchError error) noexcept
    {
      switch (error)
      {
      case ProcessLaunchError::ExecutableNotFound:
        return SoftwareOperationError::ExecutableNotFound;

      case ProcessLaunchError::PermissionDenied:
        return SoftwareOperationError::PermissionDenied;

      case ProcessLaunchError::LaunchFailed:
        return SoftwareOperationError::LaunchFailed;
      }

      return SoftwareOperationError::LaunchFailed;
    }
  } // namespace

  SoftwareManager::SoftwareManager(
      HostState &state,
      ProcessLauncher &process_launcher) noexcept
      : state_(state),
        process_launcher_(process_launcher)
  {
  }

  bool SoftwareManager::register_software(
      SoftwareId id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::optional<ProjectIdentity> project_identity)
  {
    return state_.add_software(
        SoftwareEntry(
            std::move(id),
            std::move(process_spec),
            std::move(project_identity),
            access_point));
  }

  std::optional<SoftwareEntry> SoftwareManager::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  {
    const auto *entry = state_.find_software(identity);
    return entry == nullptr ? std::nullopt : std::optional<SoftwareEntry>(*entry);
  }

  bool SoftwareManager::update_project_root(const ProjectIdentity &identity, std::string root)
  {
    auto *entry = state_.find_software(identity);
    if (entry == nullptr)
      return false;
    entry->set_working_directory(std::move(root));
    return true;
  }

  bool SoftwareManager::synchronize(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point)
  {
    auto *entry = state_.find_software(id);
    if (entry == nullptr) return false;
    const auto existing_access = entry->access_point();
    const bool access_changed = existing_access.has_value() != access_point.has_value() ||
        (existing_access.has_value() && access_point.has_value() &&
         (existing_access->protocol() != access_point->protocol() || existing_access->port() != access_point->port()));
    const bool changed = entry->process_spec().executable() != process_spec.executable() ||
                         entry->process_spec().arguments() != process_spec.arguments() ||
                         entry->process_spec().working_directory() != process_spec.working_directory() ||
                         access_changed;
    if (!changed) return false;
    if (entry->state() == SoftwareState::Running) static_cast<void>(stop(id));
    entry->set_process_spec(std::move(process_spec));
    entry->set_access_point(access_point);
    return true;
  }

  std::optional<AccessPoint> SoftwareManager::access_point(const SoftwareId &id) const noexcept
  {
    const SoftwareEntry *entry = state_.find_software(id);
    return entry == nullptr ? std::nullopt : entry->access_point();
  }

  std::vector<SoftwareEntry> SoftwareManager::software() const
  {
    return state_.software();
  }

  SoftwareOperationResult SoftwareManager::start(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return SoftwareOperationError::SoftwareUnknown;
    }

    ManagedProcess *existing = find_process(id);

    if (existing != nullptr)
    {
      if (existing->process->is_running())
      {
        return SoftwareOperationError::AlreadyRunning;
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
    entry->clear_result();

    ProcessLaunchResult launch =
        process_launcher_.launch(entry->process_spec());

    if (!launch)
    {
      const SoftwareOperationResult result(
          software_error(launch.error().value()));

      entry->set_state(SoftwareState::Failed);
      entry->set_result(result);
      return result;
    }

    std::unique_ptr<Process> process = std::move(launch).take_process();

    if (!process->is_running())
    {
      const auto code = process->exit_code();

      const SoftwareOperationResult result(
          code.has_value() && code.value() == 0
              ? SoftwareOperationError::ProcessExitedSuccessfully
              : code.has_value()
                    ? SoftwareOperationError::ProcessExitedWithNonZeroCode
                    : SoftwareOperationError::LaunchFailed,
          code);

      entry->set_state(
          code.has_value() && code.value() == 0
              ? SoftwareState::Stopped
              : SoftwareState::Failed);
      entry->set_result(result);

      return result;
    }

    processes_.push_back(
        ManagedProcess{
            entry->id(),
            std::move(process)});

    entry->set_state(SoftwareState::Running);

    return {};
  }

  SoftwareOperationResult SoftwareManager::stop(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return SoftwareOperationError::SoftwareUnknown;
    }

    ManagedProcess *managed = find_process(id);

    if (managed == nullptr)
    {
      return SoftwareOperationError::NotRunning;
    }

    if (!managed->process->stop())
    {
      entry->set_state(SoftwareState::Failed);
      const SoftwareOperationResult result(
          SoftwareOperationError::StopFailed);
      entry->set_result(result);
      return result;
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
    entry->clear_result();

    return {};
  }

  SoftwareOperationResult SoftwareManager::restart(const SoftwareId &id)
  {
    SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return SoftwareOperationError::SoftwareUnknown;
    }

    ManagedProcess *managed = find_process(id);

    if (managed != nullptr)
    {
      if (!managed->process->stop())
      {
        entry->set_state(SoftwareState::Failed);
        const SoftwareOperationResult result(
            SoftwareOperationError::StopFailed);
        entry->set_result(result);
        return result;
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
      entry->clear_result();
    }

    return start(id);
  }

  bool SoftwareManager::stop_all()
  {
    bool stopped = true;
    auto current = processes_.begin();

    while (current != processes_.end())
    {
      SoftwareEntry *entry = state_.find_software(current->id);

      if (!current->process->stop())
      {
        stopped = false;

        if (entry != nullptr)
        {
          entry->set_state(SoftwareState::Failed);
          entry->set_result(SoftwareOperationError::StopFailed);
        }

        ++current;
        continue;
      }

      if (entry != nullptr)
      {
        entry->set_state(SoftwareState::Stopped);
        entry->clear_result();
      }

      current = processes_.erase(current);
    }

    return stopped;
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
          entry->set_result(
              SoftwareOperationResult(
                  SoftwareOperationError::ProcessExitedSuccessfully,
                  code));
        }
        else
        {
          entry->set_state(SoftwareState::Failed);
          entry->set_result(
              SoftwareOperationResult(
                  SoftwareOperationError::ProcessExitedWithNonZeroCode,
                  code));
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

  std::optional<SoftwareOperationResult> SoftwareManager::result(
      const SoftwareId &id) const noexcept
  {
    const SoftwareEntry *entry = state_.find_software(id);

    if (entry == nullptr)
    {
      return std::nullopt;
    }

    return entry->result();
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
