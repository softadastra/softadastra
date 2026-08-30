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

#include "software/LocalName.hpp"

#include <utility>

namespace softadastra
{
  HostService::HostService(
      Host &host,
      ProcessLauncher &process_launcher) noexcept
      : host_(host),
        software_manager_(
            host.state(),
            process_launcher),
        connectivity_manager_(
            host.platform().network())
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
      std::optional<ProjectIdentity> project_identity,
      std::string name)
  {
    return software_manager_.register_software(
        std::move(id),
        std::move(process_spec),
        access_point,
        std::move(project_identity),
        std::move(name));
  }

  std::optional<SoftwareEntry>
  HostService::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  {
    return software_manager_.software_by_project_identity(
        identity);
  }

  bool HostService::update_project_root(
      const ProjectIdentity &identity,
      std::string root)
  {
    return software_manager_.update_project_root(
        identity,
        std::move(root));
  }

  std::optional<ProjectIdentity>
  HostService::project_identity(
      const SoftwareId &id) const noexcept
  {
    const auto *entry =
        host_.state().find_software(id);

    return entry == nullptr
               ? std::nullopt
               : entry->project_identity();
  }

  std::optional<SoftwareEntry> HostService::software(
      const SoftwareId &id) const noexcept
  {
    const auto *entry =
        host_.state().find_software(id);

    return entry == nullptr
               ? std::nullopt
               : std::optional<SoftwareEntry>(*entry);
  }

  std::optional<SoftwareEntry> HostService::find_by_name(
      const std::string &name) const noexcept
  {
    return software_manager_.find_by_name(name);
  }

  std::vector<SoftwareEntry> HostService::software() const
  {
    return software_manager_.software();
  }

  bool HostService::remove_software(
      const SoftwareId &id)
  {
    return software_manager_.remove(id);
  }

  bool HostService::link_project(
      const SoftwareId &id,
      ProjectIdentity identity,
      std::string root)
  {
    auto *entry =
        host_.state().find_software(id);

    if (entry == nullptr ||
        entry->project_identity().has_value())
    {
      return false;
    }

    if (host_.state().find_software(identity) != nullptr)
    {
      return false;
    }

    entry->set_project_identity(
        std::move(identity));

    entry->set_working_directory(
        std::move(root));

    return true;
  }

  bool HostService::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::string name)
  {
    return software_manager_.synchronize(
        id,
        std::move(process_spec),
        access_point,
        std::move(name));
  }

  bool HostService::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::vector<AccessPoint> access_points,
      std::string name)
  {
    return software_manager_.synchronize(
        id,
        std::move(process_spec),
        std::move(access_points),
        std::move(name));
  }

  SoftwareOperationResult HostService::start_software(
      const SoftwareId &id)
  {
    return software_manager_.start(id);
  }

  SoftwareOperationResult HostService::stop_software(
      const SoftwareId &id)
  {
    const auto result = software_manager_.stop(id);
    return result;
  }

  SoftwareOperationResult HostService::restart_software(
      const SoftwareId &id)
  {
    return software_manager_.restart(id);
  }

  bool HostService::shutdown()
  {
    const bool stopped =
        software_manager_.stop_all();

    const auto network =
        managed_network_status();

    const bool network_stopped =
        network.state != ManagedNetworkState::Running ||
        stop_managed_network();

    software_manager_.refresh();

    return stopped && network_stopped;
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

  std::optional<AccessPoint> HostService::access_point(
      const SoftwareId &id) const noexcept
  {
    return software_manager_.access_point(id);
  }

  LocalGatewayTarget HostService::resolve(
      std::string_view host) const
  {
    constexpr std::string_view suffix =
        ".softadastra.home.arpa";

    std::string label(host);

    if (label.ends_with(suffix))
    {
      label.resize(
          label.size() - suffix.size());
    }

    const auto local_name =
        LocalName::from_software_name(label);

    if (!local_name ||
        (host != local_name->short_name() &&
         host != local_name->canonical_name()))
    {
      return {};
    }

    const auto entry =
        software_manager_.find_by_name(label);

    if (!entry)
    {
      return {};
    }

    if (entry->state() != SoftwareState::Running)
    {
      return {
          LocalGatewayLookup::Unavailable,
          0};
    }

    const auto access =
        entry->access_point();

    if (!access ||
        access->protocol() != AccessProtocol::Http)
    {
      return {};
    }

    return {
        LocalGatewayLookup::Http,
        access->port()};
  }

  LocalReachabilityState
  HostService::local_reachability_state() const noexcept
  {
    return local_reachability_ == nullptr
               ? LocalReachabilityState::Unavailable
               : local_reachability_->state();
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
    Network &network =
        host_.platform().network();

    return LocalHostAccess{
        network.host_name(),
        network.primary_ipv4(),
        network.local_addresses()};
  }

  std::optional<LocalAccess> HostService::local_access(
      const SoftwareId &id) noexcept
  {
    const auto entry =
        software(id);

    if (!entry.has_value() ||
        !entry->access_point().has_value())
    {
      return std::nullopt;
    }

    auto access =
        resolve_local_access(
            entry->access_point().value(),
            network_capability(),
            managed_network_status(),
            entry->state() == SoftwareState::Running,
            entry->name(),
            local_reachability_state());

    if (access.state == LocalAccessState::Available &&
        access.network == LocalAccessNetwork::Existing &&
        !access.local_subnet.empty())
    {
      access.firewall = ensure_local_firewall(id, access, network_capability());
      if (access.firewall != LocalAccessFirewallState::Open)
      {
        access.state = LocalAccessState::Unavailable;
        access.url.clear();
        return access;
      }
    }

    // `access` may change networking only here: a real, running AccessPoint
    // has no usable local network and ManagedNetwork has conservatively
    // declared itself available. Existing networks and running managed
    // networks are always used as-is.
    if (access.state == LocalAccessState::Available ||
        entry->state() != SoftwareState::Running)
    {
      return access;
    }

    const auto managed =
        managed_network_status();

    if (managed.capability !=
            ManagedNetworkCapability::Available ||
        managed.state !=
            ManagedNetworkState::Stopped)
    {
      return access;
    }

    static_cast<void>(
        start_managed_network());

    access =
        resolve_local_access(
            entry->access_point().value(),
            network_capability(),
            managed_network_status(),
            true,
            entry->name(),
            local_reachability_ != nullptr
                ? local_reachability_->start()
                : LocalReachabilityState::Unavailable);

    if (access.state != LocalAccessState::Available)
    {
      access.managed_network_start_failed = true;
    }

    return access;
  }

  NetworkCapability HostService::network_capability() const
  {
    return host_.platform()
        .network()
        .network_capability();
  }

  ManagedNetworkStatus
  HostService::managed_network_status() const
  {
    return host_.platform()
        .managed_network()
        .status();
  }

  ManagedNetworkStartResult
  HostService::start_managed_network()
  {
    return host_.platform()
        .managed_network()
        .start();
  }

  bool HostService::stop_managed_network()
  {
    return host_.platform()
        .managed_network()
        .stop();
  }

  std::string HostService::primary_ipv4() const
  {
    return host_.platform()
        .network()
        .primary_ipv4();
  }

  LocalAccessFirewallState HostService::ensure_local_firewall(
      const SoftwareId &id,
      const LocalAccess &access,
      const NetworkCapability &network)
  {
    if (network.local_subnet.empty())
      return LocalAccessFirewallState::Unsupported;
    const auto result = host_.platform().local_firewall().ensure(
        {id.value(), network.local_subnet, access.port});
    switch (result)
    {
    case LocalFirewallResult::Open: return LocalAccessFirewallState::Open;
    case LocalFirewallResult::PermissionRequired: return LocalAccessFirewallState::PermissionRequired;
    case LocalFirewallResult::Unsupported: return LocalAccessFirewallState::Unsupported;
    case LocalFirewallResult::Failed: return LocalAccessFirewallState::Failed;
    }
    return LocalAccessFirewallState::Failed;
  }


} // namespace softadastra
