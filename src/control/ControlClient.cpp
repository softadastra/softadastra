/**
 *
 *  @file ControlClient.cpp
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

#include "control/ControlClient.hpp"

#include "control/LocalControlProtocol.hpp"

#include <array>
#include <cstring>
#include <functional>
#include <utility>

#if defined(__linux__)

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#endif

#if defined(_WIN32)
#include <windows.h>
#endif

namespace softadastra
{
#if defined(_WIN32)
  namespace
  {
    std::wstring pipe_name(const std::filesystem::path &path)
    {
      return L"\\\\.\\pipe\\Softadastra-" +
             std::to_wstring(std::hash<std::wstring>{}(path.wstring()));
    }
  } // namespace
#endif

  ControlClient::ControlClient(ControlServer &server) noexcept
      : server_(&server)
  {
  }

  ControlClient::ControlClient(std::filesystem::path path) noexcept
      : path_(std::move(path))
  {
  }

  bool ControlClient::host_available() const noexcept
  {
    return server_ != nullptr || request("ping").has_value();
  }

  bool ControlClient::register_software(
      SoftwareId id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::optional<ProjectIdentity> project_identity,
      std::string name)
  {
    if (server_ != nullptr)
    {
      return server_->register_software(
          std::move(id),
          std::move(process_spec),
          access_point,
          std::move(project_identity),
          std::move(name));
    }

    const std::string protocol = access_point.has_value()
                                     ? std::string(AccessPoint::name(access_point->protocol()))
                                     : "-";
    const std::string port = access_point.has_value()
                                 ? std::to_string(access_point->port())
                                 : "0";
    std::string message = "register " + LocalControlProtocol::encode(id.value()) +
                          " " + LocalControlProtocol::encode(name) +
                          " " + LocalControlProtocol::encode(process_spec.executable()) +
                          " " + LocalControlProtocol::encode(project_identity.has_value() ? project_identity->value() : "") +
                          " " + LocalControlProtocol::encode(process_spec.working_directory().value_or("")) +
                          " " + protocol + " " + port + " " +
                          std::to_string(process_spec.arguments().size());

    for (const auto &argument : process_spec.arguments())
    {
      message += " " + LocalControlProtocol::encode(argument);
    }

    const auto response = request(message);

    return response.has_value() && response.value() == "register 1";
  }

  std::optional<SoftwareEntry> ControlClient::software_by_project_identity(
      const ProjectIdentity &identity) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->software_by_project_identity(identity);
    }
    const auto response = request("project " + LocalControlProtocol::encode(identity.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    if (fields.size() != 4 || fields[0] != "project" || fields[1] != "1")
      return std::nullopt;
    const auto id = LocalControlProtocol::decode(fields[2]);
    const auto executable = LocalControlProtocol::decode(fields[3]);
    if (!id.has_value() || !executable.has_value())
    {
      return std::nullopt;
    }
    return SoftwareEntry(SoftwareId(id.value()), ProcessSpec(executable.value()), identity);
  }

  bool ControlClient::update_project_root(const ProjectIdentity &identity, std::string root)
  {
    if (server_ != nullptr)
    {
      return server_->update_project_root(identity, std::move(root));
    }
    const auto response = request(
        "project-root " + LocalControlProtocol::encode(identity.value()) +
        " " + LocalControlProtocol::encode(root));
    return response.has_value() && response.value() == "project-root 1";
  }

  std::optional<ProjectIdentity> ControlClient::project_identity(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->project_identity(id);
    }
    const auto response = request("software-project " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    if (fields.size() != 3 || fields[0] != "software-project" || fields[1] != "1")
    {
      return std::nullopt;
    }
    const auto value = LocalControlProtocol::decode(fields[2]);
    return value.has_value() && !value->empty()
               ? std::optional<ProjectIdentity>(ProjectIdentity(value.value()))
               : std::nullopt;
  }

  std::optional<SoftwareEntry> ControlClient::software(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->software(id);
    }
    const auto entries = software();
    for (const auto &entry : entries)
    {
      if (entry.id() == id)
      {
        return entry;
      }
    }
    return std::nullopt;
  }

  std::vector<SoftwareEntry> ControlClient::software() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->software();
    }
    const auto response = request("software-list-v2");
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    if (fields.size() < 2 || fields[0] != "software-list-v2")
    {
      return {};
    }
    const auto count = LocalControlProtocol::integer(fields[1]);
    if (!count || count.value() < 0)
    {
      return {};
    }
    std::vector<SoftwareEntry> entries;
    std::size_t offset = 2;
    for (std::size_t index = 0; index < static_cast<std::size_t>(*count); ++index)
    {
      if (offset + 9 > fields.size())
      {
        return {};
      }
      const auto id = LocalControlProtocol::decode(fields[offset++]);
      const auto name = LocalControlProtocol::decode(fields[offset++]);
      const auto state = LocalControlProtocol::integer(fields[offset++]);
      const auto executable = LocalControlProtocol::decode(fields[offset++]);
      const auto root = LocalControlProtocol::decode(fields[offset++]);
      const auto identity = LocalControlProtocol::decode(fields[offset++]);
      const auto declared = LocalControlProtocol::decode(fields[offset++]);
      const auto pid = LocalControlProtocol::integer(fields[offset++]);
      const auto access_count = LocalControlProtocol::integer(fields[offset++]);
      if (!id || !name || !state || !executable || !root || !identity ||
          !declared || !pid || !access_count || *access_count < 0)
      {
        return {};
      }
      std::vector<AccessPoint> accesses;
      for (int access_index = 0; access_index < *access_count; ++access_index)
      {
        if (offset + 2 > fields.size())
        {
          return {};
        }
        const auto protocol = AccessPoint::protocol(fields[offset++]);
        const auto port = LocalControlProtocol::integer(fields[offset++]);
        if (!protocol || !port || *port < 1 || *port > 65535)
        {
          return {};
        }
        const auto access = AccessPoint::create(*protocol, static_cast<std::uint16_t>(*port));
        if (!access)
        {
          return {};
        }
        accesses.push_back(*access);
      }
      if (offset >= fields.size())
      {
        return {};
      }
      const auto argument_count = LocalControlProtocol::integer(fields[offset++]);
      if (!argument_count || *argument_count < 0 ||
          offset + static_cast<std::size_t>(*argument_count) > fields.size())
      {
        return {};
      }
      std::vector<std::string> arguments;
      for (int argument_index = 0; argument_index < *argument_count; ++argument_index)
      {
        const auto argument = LocalControlProtocol::decode(fields[offset++]);
        if (!argument)
        {
          return {};
        }
        arguments.push_back(*argument);
      }
      entries.emplace_back(
          SoftwareId(*id),
          ProcessSpec(
              *executable,
              std::move(arguments),
              root->empty() ? std::nullopt : std::optional<std::string>(*root)),
          identity->empty()
              ? std::nullopt
              : std::optional<ProjectIdentity>(ProjectIdentity(*identity)),
          std::move(accesses),
          *declared,
          *name);
      if (*pid >= 0)
      {
        entries.back().set_pid(*pid);
      }
      entries.back().set_state(static_cast<SoftwareState>(*state));
    }
    if (offset != fields.size())
    {
      return {};
    }
    return entries;
  }

  bool ControlClient::remove_software(const SoftwareId &id)
  {
    if (server_ != nullptr)
    {
      return server_->remove_software(id);
    }
    const auto response = request("remove " + LocalControlProtocol::encode(id.value()));
    return response.has_value() && response.value() == "remove 1";
  }

  std::optional<std::string> ControlClient::logs(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->logs(id);
    }
    const auto response = request("logs " + LocalControlProtocol::encode(id.value()));
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    if (fields.size() != 2 || fields[0] != "logs")
    {
      return std::nullopt;
    }
    return LocalControlProtocol::decode(fields[1]);
  }

  std::optional<LogChunk> ControlClient::logs_since(
      const SoftwareId &id, std::optional<std::uintmax_t> offset) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->logs_since(id, offset);
    }
    const std::string requested_offset = offset ? std::to_string(*offset) : "-1";
    const auto response = request("logs-since " + LocalControlProtocol::encode(id.value()) + " " + requested_offset);
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    if (fields.size() != 4 || fields[0] != "logs-since")
    {
      return std::nullopt;
    }
    const auto logs = LocalControlProtocol::decode(fields[1]);
    try
    {
      const auto returned_offset = std::stoull(fields[2]);
      if (!logs || (fields[3] != "0" && fields[3] != "1"))
      {
        return std::nullopt;
      }
      return LogChunk{*logs, static_cast<std::uintmax_t>(returned_offset), fields[3] == "1"};
    }
    catch (const std::exception &)
    {
      return std::nullopt;
    }
  }

  bool ControlClient::clear_logs(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->clear_logs(id);
    }
    const auto response = request("logs-clear " + LocalControlProtocol::encode(id.value()));
    return response && *response == "logs-clear 1";
  }

  bool ControlClient::link_project(const SoftwareId &id, ProjectIdentity identity, std::string root)
  {
    if (server_ != nullptr)
    {
      return server_->link_project(id, std::move(identity), std::move(root));
    }
    const auto response = request(
        "link-project " + LocalControlProtocol::encode(id.value()) +
        " " + LocalControlProtocol::encode(identity.value()) +
        " " + LocalControlProtocol::encode(root));
    return response.has_value() && response.value() == "link-project 1";
  }

  bool ControlClient::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::optional<AccessPoint> access_point,
      std::string name)
  {
    if (server_ != nullptr)
    {
      return server_->synchronize_software(
          id,
          std::move(process_spec),
          access_point,
          std::move(name));
    }
    const std::string protocol = access_point.has_value()
                                     ? std::string(AccessPoint::name(access_point->protocol()))
                                     : "-";
    const std::string port = access_point.has_value()
                                 ? std::to_string(access_point->port())
                                 : "0";
    std::string message = "sync " + LocalControlProtocol::encode(id.value()) +
                          " " + LocalControlProtocol::encode(name) +
                          " " + LocalControlProtocol::encode(process_spec.executable()) +
                          " " + LocalControlProtocol::encode(process_spec.working_directory().value_or("")) +
                          " " + protocol +
                          " " + port +
                          " " + std::to_string(process_spec.arguments().size());
    for (const auto &argument : process_spec.arguments())
    {
      message += " " + LocalControlProtocol::encode(argument);
    }
    const auto response = request(message);
    return response.has_value() && response.value() == "sync 1";
  }

  bool ControlClient::synchronize_software(
      const SoftwareId &id,
      ProcessSpec process_spec,
      std::vector<AccessPoint> access_points,
      std::string name)
  {
    if (server_ != nullptr)
    {
      return server_->synchronize_software(
          id,
          std::move(process_spec),
          std::move(access_points),
          std::move(name));
    }
    std::string message = "sync-v2 " + LocalControlProtocol::encode(id.value()) +
                          " " + LocalControlProtocol::encode(name) +
                          " " + LocalControlProtocol::encode(process_spec.executable()) +
                          " " + LocalControlProtocol::encode(process_spec.working_directory().value_or("")) +
                          " " + std::to_string(access_points.size());
    for (const auto &access : access_points)
    {
      message += " " + std::string(AccessPoint::name(access.protocol())) +
                 " " + std::to_string(access.port());
    }
    message += " " + std::to_string(process_spec.arguments().size());
    for (const auto &argument : process_spec.arguments())
    {
      message += " " + LocalControlProtocol::encode(argument);
    }
    const auto response = request(message);
    return response && *response == "sync-v2 1";
  }

  std::optional<AccessPoint> ControlClient::access_point(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
      return server_->access_point(id);

    const auto response = request("access-point " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value() ? LocalControlProtocol::fields(response.value())
                                             : std::vector<std::string>{};
    if (fields.size() != 4 || fields[0] != "access-point" || fields[1] != "1")
      return std::nullopt;
    const auto protocol = AccessPoint::protocol(fields[2]);
    const auto port = LocalControlProtocol::integer(fields[3]);
    if (!protocol.has_value() || !port.has_value() || port.value() < 1 || port.value() > 65535)
      return std::nullopt;
    return AccessPoint::create(protocol.value(), static_cast<std::uint16_t>(port.value()));
  }

  LocalGatewayTarget ControlClient::local_gateway_target(std::string_view host) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->local_gateway_target(host);
    }
    const auto response = request("local-gateway-target " + LocalControlProtocol::encode(host));
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    if (fields.size() == 1 && fields[0] == "not-found")
    {
      return {};
    }
    if (fields.size() == 1 && fields[0] == "unavailable")
    {
      return {LocalGatewayLookup::Unavailable, 0};
    }
    const auto port = fields.size() == 2 && fields[0] == "http"
                          ? LocalControlProtocol::integer(fields[1])
                          : std::nullopt;
    if (!port || *port < 1 || *port > 65535)
    {
      return {};
    }
    return {LocalGatewayLookup::Http, static_cast<std::uint16_t>(*port)};
  }

  std::optional<LocalReachabilityState> ControlClient::local_reachability_state() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->local_reachability_state();
    }
    const auto response = request("local-reachability");
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    const auto state = fields.size() == 2 && fields[0] == "local-reachability"
                           ? LocalControlProtocol::integer(fields[1])
                           : std::nullopt;
    if (!state || *state < 0 ||
        *state > static_cast<int>(LocalReachabilityState::Degraded))
    {
      return std::nullopt;
    }
    return static_cast<LocalReachabilityState>(*state);
  }

  SoftwareOperationResult ControlClient::start_software(const SoftwareId &id)
  {
    if (server_ != nullptr)
    {
      return server_->start_software(id);
    }

    const auto response = request("start " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    const auto error = fields.size() == 4 ? LocalControlProtocol::integer(fields[1]) : std::nullopt;
    const auto has_code = fields.size() == 4 ? LocalControlProtocol::integer(fields[2]) : std::nullopt;
    const auto code = fields.size() == 4 ? LocalControlProtocol::integer(fields[3]) : std::nullopt;

    if (fields.empty() || fields[0] != "operation" || !error.has_value() ||
        !has_code.has_value() || !code.has_value())
    {
      return SoftwareOperationError::LaunchFailed;
    }

    const int error_code = *error;
    const int exit_code = *code;
    return error_code < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error_code),
                     *has_code != 0 ? std::optional<int>(exit_code) : std::nullopt);
  }

  SoftwareOperationResult ControlClient::stop_software(const SoftwareId &id)
  {
    if (server_ != nullptr)
    {
      return server_->stop_software(id);
    }

    const auto response = request("stop " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    const auto error = fields.size() == 4 ? LocalControlProtocol::integer(fields[1]) : std::nullopt;
    const auto has_code = fields.size() == 4 ? LocalControlProtocol::integer(fields[2]) : std::nullopt;
    const auto code = fields.size() == 4 ? LocalControlProtocol::integer(fields[3]) : std::nullopt;

    if (fields.empty() || fields[0] != "operation" || !error.has_value() ||
        !has_code.has_value() || !code.has_value())
    {
      return SoftwareOperationError::LaunchFailed;
    }

    const int error_code = *error;
    const int exit_code = *code;
    return error_code < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error_code),
                     *has_code != 0 ? std::optional<int>(exit_code) : std::nullopt);
  }

  SoftwareOperationResult ControlClient::restart_software(const SoftwareId &id)
  {
    if (server_ != nullptr)
    {
      return server_->restart_software(id);
    }

    const auto response = request("restart " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    const auto error = fields.size() == 4 ? LocalControlProtocol::integer(fields[1]) : std::nullopt;
    const auto has_code = fields.size() == 4 ? LocalControlProtocol::integer(fields[2]) : std::nullopt;
    const auto code = fields.size() == 4 ? LocalControlProtocol::integer(fields[3]) : std::nullopt;

    if (fields.empty() || fields[0] != "operation" || !error.has_value() ||
        !has_code.has_value() || !code.has_value())
    {
      return SoftwareOperationError::LaunchFailed;
    }

    const int error_code = *error;
    const int exit_code = *code;
    return error_code < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error_code),
                     *has_code != 0 ? std::optional<int>(exit_code) : std::nullopt);
  }

  void ControlClient::refresh()
  {
    if (server_ != nullptr)
    {
      server_->refresh();
    }
  }

  std::optional<SoftwareState> ControlClient::software_state(
      const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->software_state(id);
    }

    const auto response = request("status " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    const auto state = fields.size() == 5 ? LocalControlProtocol::integer(fields[1]) : std::nullopt;

    if (fields.empty() || fields[0] != "status" || !state.has_value())
    {
      return std::nullopt;
    }

    const int state_value = *state;
    if (state_value < 0)
    {
      return std::nullopt;
    }
    return static_cast<SoftwareState>(state_value);
  }

  std::optional<SoftwareOperationResult> ControlClient::software_result(
      const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->software_result(id);
    }

    const auto response = request("status " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    const auto error = fields.size() == 5 ? LocalControlProtocol::integer(fields[2]) : std::nullopt;
    const auto has_code = fields.size() == 5 ? LocalControlProtocol::integer(fields[3]) : std::nullopt;
    const auto code = fields.size() == 5 ? LocalControlProtocol::integer(fields[4]) : std::nullopt;

    if (fields.empty() || fields[0] != "status" || !error.has_value() ||
        !has_code.has_value() || !code.has_value())
    {
      return std::nullopt;
    }

    const int error_code = *error;
    const int exit_code = *code;
    return error_code < 0
               ? std::optional<SoftwareOperationResult>()
               : std::optional<SoftwareOperationResult>(SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error_code),
                     *has_code != 0 ? std::optional<int>(exit_code)
                                    : std::nullopt));
  }

  bool ControlClient::connectivity_available() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->connectivity_available();
    }

    const auto response = request("connectivity");
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    return fields.size() == 3 && fields[0] == "connectivity" && fields[1] == "1";
  }

  bool ControlClient::connected() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->connected();
    }

    const auto response = request("connectivity");
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    return fields.size() == 3 && fields[0] == "connectivity" && fields[2] == "1";
  }

  std::optional<LocalHostAccess> ControlClient::local_access() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->local_access();
    }

    const auto response = request("access");
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};

    if (fields.size() < 4 || fields[0] != "access")
    {
      return std::nullopt;
    }

    const auto name = LocalControlProtocol::decode(fields[1]);
    const auto primary_ipv4 = LocalControlProtocol::decode(fields[2]);
    const auto count = LocalControlProtocol::integer(fields[3]);

    if (!name.has_value() || !primary_ipv4.has_value() ||
        !count.has_value() || count.value() < 0 ||
        fields.size() != static_cast<std::size_t>(count.value()) * 3 + 4)
    {
      return std::nullopt;
    }

    LocalHostAccess access{name.value(), primary_ipv4.value(), {}};
    access.addresses.reserve(static_cast<std::size_t>(count.value()));

    for (std::size_t index = 4; index < fields.size(); index += 3)
    {
      const auto family = LocalControlProtocol::integer(fields[index]);
      const auto interface_name = LocalControlProtocol::decode(fields[index + 1]);
      const auto value = LocalControlProtocol::decode(fields[index + 2]);

      if (!family.has_value() || !interface_name.has_value() ||
          !value.has_value() || (family.value() != 4 && family.value() != 6))
      {
        return std::nullopt;
      }

      access.addresses.push_back(
          LocalNetworkAddress{
              family.value() == 4 ? LocalAddressFamily::IPv4
                                  : LocalAddressFamily::IPv6,
              interface_name.value(),
              value.value()});
    }

    return access;
  }

  std::optional<LocalAccess> ControlClient::local_access(
      const SoftwareId &id) noexcept
  {
    if (server_ != nullptr)
    {
      return server_->local_access(id);
    }

    const auto response = request("local-access " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};
    if (fields.size() != 10 || fields[0] != "local-access")
    {
      return std::nullopt;
    }

    const auto state = LocalControlProtocol::integer(fields[1]);
    const auto protocol = LocalControlProtocol::integer(fields[2]);
    const auto port = LocalControlProtocol::integer(fields[3]);
    const auto ipv4 = LocalControlProtocol::decode(fields[4]);
    const auto url = LocalControlProtocol::decode(fields[5]);
    const auto network = LocalControlProtocol::integer(fields[6]);
    const auto local_network = LocalControlProtocol::integer(fields[7]);
    const auto managed_network = LocalControlProtocol::integer(fields[8]);
    const auto start_failed = LocalControlProtocol::integer(fields[9]);
    if (!state.has_value() || !protocol.has_value() || !port.has_value() ||
        !ipv4.has_value() || !url.has_value() || !network.has_value() || !local_network.has_value() ||
        !managed_network.has_value() || state.value() < 0 || state.value() > 1 ||
        protocol.value() < 0 || protocol.value() > 1 || port.value() < 1 ||
        port.value() > 65535 || network.value() < 0 || network.value() > 2 || local_network.value() < 0 ||
        local_network.value() > 1 || managed_network.value() < 0 ||
        managed_network.value() > 1 || !start_failed.has_value() ||
        (start_failed.value() != 0 && start_failed.value() != 1))
    {
      return std::nullopt;
    }

    return LocalAccess{
        static_cast<LocalAccessState>(state.value()),
        static_cast<AccessProtocol>(protocol.value()),
        static_cast<std::uint16_t>(port.value()),
        ipv4.value(),
        url.value(),
        static_cast<LocalAccessNetwork>(network.value()),
        static_cast<LocalNetworkState>(local_network.value()),
        static_cast<ManagedNetworkCapability>(managed_network.value()),
        start_failed.value() != 0};
  }

  std::optional<NetworkCapability> ControlClient::network_capability() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->network_capability();
    }

    const auto response = request("network-capability");
    const auto fields = response.has_value()
                            ? LocalControlProtocol::fields(response.value())
                            : std::vector<std::string>{};

    if (fields.size() != 7 || fields[0] != "network-capability")
    {
      return std::nullopt;
    }

    const auto state = LocalControlProtocol::integer(fields[1]);
    const auto ipv4 = LocalControlProtocol::decode(fields[2]);
    const auto interface_name = LocalControlProtocol::decode(fields[3]);
    const auto interface_type = LocalControlProtocol::integer(fields[4]);
    const auto local_network = LocalControlProtocol::integer(fields[5]);
    const auto managed_network = LocalControlProtocol::integer(fields[6]);

    if (!state.has_value() || !ipv4.has_value() || !interface_name.has_value() ||
        !interface_type.has_value() || !local_network.has_value() ||
        !managed_network.has_value() || state.value() < 0 || state.value() > 1 ||
        interface_type.value() < 0 || interface_type.value() > 4 ||
        local_network.value() < 0 || local_network.value() > 1 ||
        managed_network.value() < 0 || managed_network.value() > 1)
    {
      return std::nullopt;
    }

    return NetworkCapability{
        static_cast<NetworkState>(state.value()),
        ipv4.value(),
        interface_name.value(),
        static_cast<NetworkInterfaceType>(interface_type.value()),
        static_cast<LocalNetworkState>(local_network.value()),
        static_cast<ManagedNetworkCapability>(managed_network.value())};
  }

  std::optional<ManagedNetworkStatus> ControlClient::managed_network_status() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->managed_network_status();
    }

    const auto response = request("managed-network-status");
    const auto fields = response
                            ? LocalControlProtocol::fields(*response)
                            : std::vector<std::string>{};

    if (fields.size() != 6 || fields[0] != "managed-network-status")
    {
      return std::nullopt;
    }

    const auto capability = LocalControlProtocol::integer(fields[1]);
    const auto state = LocalControlProtocol::integer(fields[2]);
    const auto interface_name = LocalControlProtocol::decode(fields[3]);
    const auto ipv4 = LocalControlProtocol::decode(fields[4]);
    const auto ssid = LocalControlProtocol::decode(fields[5]);

    if (!capability || !state || !interface_name || !ipv4 || !ssid ||
        *capability < 0 || *capability > 1 || *state < 0 || *state > 1)
    {
      return std::nullopt;
    }

    return ManagedNetworkStatus{
        static_cast<ManagedNetworkCapability>(*capability),
        static_cast<ManagedNetworkState>(*state),
        *interface_name,
        *ipv4,
        *ssid};
  }

  std::optional<ManagedNetworkStartResult> ControlClient::start_managed_network() const noexcept
  {
    if (server_ != nullptr)
    {
      return server_->start_managed_network();
    }
    const auto response = request("managed-network-start");
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    const auto result = fields.size() == 2 ? LocalControlProtocol::integer(fields[1]) : std::nullopt;
    if (!result || fields[0] != "managed-network-start" ||
        *result < 0 || *result > 4)
    {
      return std::nullopt;
    }
    return static_cast<ManagedNetworkStartResult>(*result);
  }

  std::optional<bool> ControlClient::stop_managed_network() const noexcept
  {
    if (server_ != nullptr)
    {
      const auto state = server_->managed_network_status();
      if (state.state != ManagedNetworkState::Running)
      {
        return false;
      }

      return server_->stop_managed_network();
    }

    const auto response = request("managed-network-stop");
    const auto fields = response
                            ? LocalControlProtocol::fields(*response)
                            : std::vector<std::string>{};

    if (fields.size() != 2 || fields[0] != "managed-network-stop")
    {
      return std::nullopt;
    }

    const auto stopped = LocalControlProtocol::integer(fields[1]);
    if (!stopped || (*stopped != 0 && *stopped != 1))
    {
      return std::nullopt;
    }

    return *stopped == 1;
  }

  std::optional<std::string> ControlClient::request(
      const std::string &message) const noexcept
  {
#if defined(__linux__)
    if (path_.string().size() >= sizeof(sockaddr_un::sun_path))
    {
      return std::nullopt;
    }

    const int descriptor = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);

    if (descriptor < 0)
    {
      return std::nullopt;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, path_.c_str(), sizeof(address.sun_path) - 1);

    if (::connect(descriptor, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0 ||
        ::send(descriptor, message.data(), message.size(), MSG_NOSIGNAL) < 0)
    {
      ::close(descriptor);
      return std::nullopt;
    }

    std::array<char, 16384> buffer{};
    const ssize_t received = ::recv(descriptor, buffer.data(), buffer.size(), 0);
    ::close(descriptor);

    if (received <= 0)
    {
      return std::nullopt;
    }

    return std::string(buffer.data(), static_cast<std::size_t>(received));
#else
#if defined(_WIN32)
    const auto name = pipe_name(path_);

    if (!::WaitNamedPipeW(name.c_str(), 50))
    {
      return std::nullopt;
    }

    HANDLE pipe = ::CreateFileW(
        name.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr);

    if (pipe == INVALID_HANDLE_VALUE)
    {
      return std::nullopt;
    }

    DWORD written{};
    if (!::WriteFile(
            pipe,
            message.data(),
            static_cast<DWORD>(message.size()),
            &written,
            nullptr))
    {
      ::CloseHandle(pipe);
      return std::nullopt;
    }

    std::array<char, 16384> buffer{};
    DWORD received{};
    const bool ok = ::ReadFile(
        pipe,
        buffer.data(),
        static_cast<DWORD>(buffer.size()),
        &received,
        nullptr);

    ::CloseHandle(pipe);

    if (!ok || received == 0)
    {
      return std::nullopt;
    }

    return std::string(buffer.data(), received);
#else
    static_cast<void>(message);
    return std::nullopt;
#endif
#endif
  }

} // namespace softadastra
