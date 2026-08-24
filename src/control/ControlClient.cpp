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
#include <utility>

#if defined(__linux__)

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#endif

namespace softadastra
{
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
      std::optional<ProjectIdentity> project_identity)
  {
    if (server_ != nullptr)
    {
      return server_->register_software(
          std::move(id),
          std::move(process_spec), access_point, std::move(project_identity));
    }

    const std::string protocol = access_point.has_value()
                                     ? std::string(AccessPoint::name(access_point->protocol()))
                                     : "-";
    const std::string port = access_point.has_value()
                                 ? std::to_string(access_point->port())
                                 : "0";
    std::string message = "register " + LocalControlProtocol::encode(id.value()) +
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
      return server_->software_by_project_identity(identity);
    const auto response = request("project " + LocalControlProtocol::encode(identity.value()));
    const auto fields = response.has_value() ? LocalControlProtocol::fields(response.value()) : std::vector<std::string>{};
    if (fields.size() != 4 || fields[0] != "project" || fields[1] != "1")
      return std::nullopt;
    const auto id = LocalControlProtocol::decode(fields[2]);
    const auto executable = LocalControlProtocol::decode(fields[3]);
    if (!id.has_value() || !executable.has_value()) return std::nullopt;
    return SoftwareEntry(SoftwareId(id.value()), ProcessSpec(executable.value()), identity);
  }

  bool ControlClient::update_project_root(const ProjectIdentity &identity, std::string root)
  {
    if (server_ != nullptr)
      return server_->update_project_root(identity, std::move(root));
    const auto response = request("project-root " + LocalControlProtocol::encode(identity.value()) + " " + LocalControlProtocol::encode(root));
    return response.has_value() && response.value() == "project-root 1";
  }

  std::optional<ProjectIdentity> ControlClient::project_identity(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr) return server_->project_identity(id);
    const auto response = request("software-project " + LocalControlProtocol::encode(id.value()));
    const auto fields = response.has_value() ? LocalControlProtocol::fields(response.value()) : std::vector<std::string>{};
    if (fields.size() != 3 || fields[0] != "software-project" || fields[1] != "1") return std::nullopt;
    const auto value = LocalControlProtocol::decode(fields[2]);
    return value.has_value() && !value->empty() ? std::optional<ProjectIdentity>(ProjectIdentity(value.value())) : std::nullopt;
  }

  std::optional<SoftwareEntry> ControlClient::software(const SoftwareId &id) const noexcept
  {
    if (server_ != nullptr) return server_->software(id);
    const auto entries = software();
    for (const auto &entry : entries)
      if (entry.id() == id) return entry;
    return std::nullopt;
  }

  std::vector<SoftwareEntry> ControlClient::software() const noexcept
  {
    if (server_ != nullptr) return server_->software();
    const auto response = request("software-list");
    const auto fields = response ? LocalControlProtocol::fields(*response) : std::vector<std::string>{};
    if (fields.size() < 2 || fields[0] != "software-list") return {};
    const auto count = LocalControlProtocol::integer(fields[1]);
    if (!count || count.value() < 0 || fields.size() != 2 + static_cast<std::size_t>(count.value()) * 9) return {};
    std::vector<SoftwareEntry> entries;
    for (std::size_t index = 0; index < static_cast<std::size_t>(count.value()); ++index)
    {
      const auto base = 2 + index * 9;
      const auto id = LocalControlProtocol::decode(fields[base]);
      const auto executable = LocalControlProtocol::decode(fields[base + 2]);
      const auto root = LocalControlProtocol::decode(fields[base + 3]);
      const auto identity = LocalControlProtocol::decode(fields[base + 4]);
      const auto declared = LocalControlProtocol::decode(fields[base + 5]);
      const auto pid = LocalControlProtocol::integer(fields[base + 6]);
      const auto protocol = fields[base + 7] == "-" ? std::optional<AccessProtocol>{} : AccessPoint::protocol(fields[base + 7]);
      const auto port = LocalControlProtocol::integer(fields[base + 8]);
      if (!id || !executable || !root || !identity || !declared || !pid || !port || port.value() < 0 || port.value() > 65535 || (fields[base + 7] != "-" && !protocol)) return {};
      const auto access = protocol ? AccessPoint::create(*protocol, static_cast<std::uint16_t>(*port)) : std::nullopt;
      entries.emplace_back(SoftwareId(*id), ProcessSpec(*executable, {}, root->empty() ? std::nullopt : std::optional<std::string>(*root)), identity->empty() ? std::nullopt : std::optional<ProjectIdentity>(ProjectIdentity(*identity)), access, *declared);
      if (*pid >= 0) entries.back().set_pid(*pid);
      entries.back().set_state(static_cast<SoftwareState>(std::stoi(fields[base + 1])));
    }
    return entries;
  }

  bool ControlClient::link_project(const SoftwareId &id, ProjectIdentity identity, std::string root)
  {
    if (server_ != nullptr) return server_->link_project(id, std::move(identity), std::move(root));
    const auto response = request("link-project " + LocalControlProtocol::encode(id.value()) + " " + LocalControlProtocol::encode(identity.value()) + " " + LocalControlProtocol::encode(root));
    return response.has_value() && response.value() == "link-project 1";
  }

  bool ControlClient::synchronize_software(const SoftwareId &id, ProcessSpec process_spec, std::optional<AccessPoint> access_point)
  {
    if (server_ != nullptr) return server_->synchronize_software(id, std::move(process_spec), access_point);
    const std::string protocol = access_point.has_value() ? std::string(AccessPoint::name(access_point->protocol())) : "-";
    const std::string port = access_point.has_value() ? std::to_string(access_point->port()) : "0";
    std::string message = "sync " + LocalControlProtocol::encode(id.value()) + " " + LocalControlProtocol::encode(process_spec.executable()) + " " + LocalControlProtocol::encode(process_spec.working_directory().value_or("")) + " " + protocol + " " + port + " " + std::to_string(process_spec.arguments().size());
    for (const auto &argument : process_spec.arguments()) message += " " + LocalControlProtocol::encode(argument);
    const auto response = request(message);
    return response.has_value() && response.value() == "sync 1";
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

    return error.value() < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error.value()),
                     has_code.value() != 0 ? std::optional<int>(code.value()) : std::nullopt);
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

    return error.value() < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error.value()),
                     has_code.value() != 0 ? std::optional<int>(code.value()) : std::nullopt);
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

    return error.value() < 0
               ? SoftwareOperationResult()
               : SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error.value()),
                     has_code.value() != 0 ? std::optional<int>(code.value()) : std::nullopt);
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

    if (fields.empty() || fields[0] != "status" || !state.has_value() || state.value() < 0)
    {
      return std::nullopt;
    }

    return static_cast<SoftwareState>(state.value());
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

    return error.value() < 0
               ? std::optional<SoftwareOperationResult>()
               : std::optional<SoftwareOperationResult>(SoftwareOperationResult(
                     static_cast<SoftwareOperationError>(error.value()),
                     has_code.value() != 0 ? std::optional<int>(code.value())
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
    static_cast<void>(message);
    return std::nullopt;
#endif
  }

} // namespace softadastra
