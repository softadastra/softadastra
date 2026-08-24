/**
 *
 *  @file ControlServer.cpp
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

#include "control/ControlServer.hpp"

#include <utility>

namespace softadastra
{
  ControlServer::ControlServer(HostService &host_service) noexcept
      : host_service_(host_service)
  {
  }

  bool ControlServer::register_software(
      SoftwareId id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::optional<ProjectIdentity> project_identity)
  {
    return host_service_.register_software(
        std::move(id),
        std::move(process_spec), access_point, std::move(project_identity));
  }

  std::optional<SoftwareEntry> ControlServer::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  { return host_service_.software_by_project_identity(identity); }

  bool ControlServer::update_project_root(const ProjectIdentity &identity, std::string root)
  { return host_service_.update_project_root(identity, std::move(root)); }

  std::optional<ProjectIdentity> ControlServer::project_identity(const SoftwareId &id) const noexcept
  { return host_service_.project_identity(id); }

  std::optional<SoftwareEntry> ControlServer::software(const SoftwareId &id) const noexcept
  { return host_service_.software(id); }

  std::vector<SoftwareEntry> ControlServer::software() const
  { return host_service_.software(); }

  bool ControlServer::link_project(const SoftwareId &id, ProjectIdentity identity, std::string root)
  { return host_service_.link_project(id, std::move(identity), std::move(root)); }

  bool ControlServer::synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point)
  { return host_service_.synchronize_software(id, std::move(process_spec), access_point); }

  std::optional<AccessPoint> ControlServer::access_point(const SoftwareId &id) const noexcept
  {
    return host_service_.access_point(id);
  }

  SoftwareOperationResult ControlServer::start_software(const SoftwareId &id)
  {
    return host_service_.start_software(id);
  }

  SoftwareOperationResult ControlServer::stop_software(const SoftwareId &id)
  {
    return host_service_.stop_software(id);
  }

  SoftwareOperationResult ControlServer::restart_software(const SoftwareId &id)
  {
    return host_service_.restart_software(id);
  }

  void ControlServer::refresh()
  {
    host_service_.refresh();
  }

  std::optional<SoftwareState> ControlServer::software_state(
      const SoftwareId &id) const noexcept
  {
    return host_service_.software_state(id);
  }

  std::optional<SoftwareOperationResult> ControlServer::software_result(
      const SoftwareId &id) const noexcept
  {
    return host_service_.software_result(id);
  }

  bool ControlServer::connectivity_available() const noexcept
  {
    return host_service_.connectivity_available();
  }

  bool ControlServer::connected() const noexcept
  {
    return host_service_.connected();
  }

  LocalHostAccess ControlServer::local_access() const
  {
    return host_service_.local_access();
  }

} // namespace softadastra
