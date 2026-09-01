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
#include "platform/NativeDataDirectory.hpp"

#include <filesystem>
#include <fstream>
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
      std::optional<ProjectIdentity> project_identity,
      std::string name)
  {
    return host_service_.register_software(
        std::move(id),
        std::move(process_spec),
        access_point,
        std::move(project_identity),
        std::move(name));
  }

  std::optional<SoftwareEntry> ControlServer::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  {
    return host_service_.software_by_project_identity(identity);
  }

  bool ControlServer::update_project_root(
      const ProjectIdentity &identity,
      std::string root)
  {
    return host_service_.update_project_root(identity, std::move(root));
  }

  std::optional<ProjectIdentity> ControlServer::project_identity(
      const SoftwareId &id) const noexcept
  {
    return host_service_.project_identity(id);
  }

  std::optional<SoftwareEntry> ControlServer::software(
      const SoftwareId &id) const noexcept
  {
    return host_service_.software(id);
  }

  std::vector<SoftwareEntry> ControlServer::software() const
  {
    return host_service_.software();
  }

  bool ControlServer::remove_software(const SoftwareId &id)
  {
    return host_service_.remove_software(id);
  }

  std::optional<std::string> ControlServer::logs(
      const SoftwareId &id) const noexcept
  {
    const auto chunk = logs_since(id, std::nullopt);

    return chunk
               ? std::optional<std::string>(chunk->logs)
               : std::nullopt;
  }

  std::optional<LogChunk> ControlServer::logs_since(
      const SoftwareId &id,
      std::optional<std::uintmax_t> offset) const noexcept
  {
    if (!host_service_.software(id))
    {
      return std::nullopt;
    }

    const auto path =
        NativeDataDirectory::path() / "logs" / (id.value() + ".log");

    std::error_code error;
    const auto size = std::filesystem::file_size(path, error);

    if (error)
    {
      return LogChunk{
          "",
          0,
          offset.has_value() && offset.value() != 0};
    }

    constexpr std::uintmax_t initial_maximum = 4096;

    const bool reset =
        offset.has_value() && offset.value() > size;

    const std::uintmax_t start =
        !offset.has_value()
            ? (size > initial_maximum ? size - initial_maximum : 0)
            : (reset ? 0 : offset.value());

    std::ifstream input(path, std::ios::binary);

    if (!input)
    {
      return LogChunk{
          "",
          0,
          offset.has_value() && offset.value() != 0};
    }

    input.seekg(static_cast<std::streamoff>(start));

    std::string logs(
        static_cast<std::size_t>(size - start),
        '\0');

    input.read(
        logs.data(),
        static_cast<std::streamsize>(logs.size()));

    logs.resize(
        static_cast<std::size_t>(input.gcount()));

    return LogChunk{
        std::move(logs),
        size,
        reset};
  }

  bool ControlServer::clear_logs(
      const SoftwareId &id) const noexcept
  {
    if (!host_service_.software(id))
    {
      return false;
    }

    std::ofstream output(
        NativeDataDirectory::path() / "logs" / (id.value() + ".log"),
        std::ios::trunc);

    return static_cast<bool>(output);
  }

  bool ControlServer::link_project(
      const SoftwareId &id,
      ProjectIdentity identity,
      std::string root)
  {
    return host_service_.link_project(
        id,
        std::move(identity),
        std::move(root));
  }

  bool ControlServer::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::string name)
  {
    return host_service_.synchronize_software(
        id,
        std::move(process_spec),
        access_point,
        std::move(name));
  }

  bool ControlServer::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::vector<AccessPoint> access_points,
      std::string name)
  {
    return host_service_.synchronize_software(
        id,
        std::move(process_spec),
        std::move(access_points),
        std::move(name));
  }

  std::optional<AccessPoint> ControlServer::access_point(
      const SoftwareId &id) const noexcept
  {
    return host_service_.access_point(id);
  }

  LocalGatewayTarget ControlServer::local_gateway_target(
      std::string_view host) const
  {
    return host_service_.resolve(host);
  }

  LocalReachabilityState ControlServer::local_reachability_state() const noexcept
  {
    return host_service_.local_reachability_state();
  }

  SoftwareOperationResult ControlServer::start_software(
      const SoftwareId &id)
  {
    return host_service_.start_software(id);
  }

  SoftwareOperationResult ControlServer::stop_software(
      const SoftwareId &id)
  {
    return host_service_.stop_software(id);
  }

  SoftwareOperationResult ControlServer::restart_software(
      const SoftwareId &id)
  {
    return host_service_.restart_software(id);
  }

  void ControlServer::refresh()
  {
    host_service_.refresh();
  }

  std::optional<SoftwareState> ControlServer::software_state(
      const SoftwareId &id) noexcept
  {
    host_service_.refresh();
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

  std::optional<LocalAccess> ControlServer::local_access(
      const SoftwareId &id) noexcept
  {
    return host_service_.local_access(id);
  }

  NetworkCapability ControlServer::network_capability() const
  {
    return host_service_.network_capability();
  }

  ManagedNetworkStatus ControlServer::managed_network_status() const
  {
    return host_service_.managed_network_status();
  }

  ManagedNetworkStartResult ControlServer::start_managed_network()
  {
    return host_service_.start_managed_network();
  }

  bool ControlServer::stop_managed_network()
  {
    return host_service_.stop_managed_network();
  }

} // namespace softadastra
