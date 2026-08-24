/**
 *
 *  @file HostService.cpp
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

#include "host/HostService.hpp"

#include <utility>

namespace softadastra
{
  HostService::HostService(
      Host &host,
      ProcessLauncher &process_launcher) noexcept
      : host_(host),
        software_manager_(host.state(), process_launcher),
        connectivity_manager_(host.platform().network())
  {
  }

  Host &HostService::host() noexcept
  {
    return host_;
  }

  const Host &HostService::host() const noexcept
  {
    return host_;
  }

  bool HostService::register_software(
      SoftwareId id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::optional<ProjectIdentity> project_identity)
  {
    return software_manager_.register_software(
        std::move(id),
        std::move(process_spec), access_point, std::move(project_identity));
  }

  std::optional<SoftwareEntry> HostService::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  { return software_manager_.software_by_project_identity(identity); }

  bool HostService::update_project_root(const ProjectIdentity &identity, std::string root)
  { return software_manager_.update_project_root(identity, std::move(root)); }

  std::optional<ProjectIdentity> HostService::project_identity(const SoftwareId &id) const noexcept
  {
    const auto *entry = host_.state().find_software(id);
    return entry == nullptr ? std::nullopt : entry->project_identity();
  }

  std::optional<SoftwareEntry> HostService::software(const SoftwareId &id) const noexcept
  {
    const auto *entry = host_.state().find_software(id);
    return entry == nullptr ? std::nullopt : std::optional<SoftwareEntry>(*entry);
  }

  std::vector<SoftwareEntry> HostService::software() const
  { return software_manager_.software(); }
  bool HostService::remove_software(const SoftwareId &id) { return software_manager_.remove(id); }

  bool HostService::link_project(const SoftwareId &id, ProjectIdentity identity, std::string root)
  {
    auto *entry = host_.state().find_software(id);
    if (entry == nullptr || entry->project_identity().has_value()) return false;
    if (host_.state().find_software(identity) != nullptr) return false;
    entry->set_project_identity(std::move(identity));
    entry->set_working_directory(std::move(root));
    return true;
  }

  bool HostService::synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point)
  { return software_manager_.synchronize(id, std::move(process_spec), access_point); }

  SoftwareOperationResult HostService::start_software(const SoftwareId &id)
  {
    return software_manager_.start(id);
  }

  SoftwareOperationResult HostService::stop_software(const SoftwareId &id)
  {
    return software_manager_.stop(id);
  }

  SoftwareOperationResult HostService::restart_software(const SoftwareId &id)
  {
    return software_manager_.restart(id);
  }

  bool HostService::shutdown()
  {
    const bool stopped = software_manager_.stop_all();
    software_manager_.refresh();
    return stopped;
  }

  void HostService::refresh()
  {
    software_manager_.refresh();
  }

  std::optional<SoftwareState> HostService::software_state(
      const SoftwareId &id) const noexcept
  {
    return software_manager_.state(id);
  }

  std::optional<SoftwareOperationResult> HostService::software_result(
      const SoftwareId &id) const noexcept
  {
    return software_manager_.result(id);
  }

  std::optional<AccessPoint> HostService::access_point(const SoftwareId &id) const noexcept
  {
    return software_manager_.access_point(id);
  }

  bool HostService::connectivity_available() const noexcept
  {
    return connectivity_manager_.is_available();
  }

  bool HostService::connected() const noexcept
  {
    return connectivity_manager_.is_connected();
  }

  LocalHostAccess HostService::local_access() const
  {
    Network &network = host_.platform().network();
    return LocalHostAccess{
        network.host_name(),
        network.primary_ipv4(),
        network.local_addresses()};
  }

  NetworkCapability HostService::network_capability() const
  {
    return host_.platform().network().network_capability();
  }

  std::string HostService::primary_ipv4() const
  {
    return host_.platform().network().primary_ipv4();
  }

} // namespace softadastra
