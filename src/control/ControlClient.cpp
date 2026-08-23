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
      ProcessSpec process_spec)
  {
    if (server_ != nullptr)
    {
      return server_->register_software(
          std::move(id),
          std::move(process_spec));
    }

    std::string message = "register " + LocalControlProtocol::encode(id.value()) +
                          " " + LocalControlProtocol::encode(process_spec.executable()) +
                          " " + std::to_string(process_spec.arguments().size());

    for (const auto &argument : process_spec.arguments())
    {
      message += " " + LocalControlProtocol::encode(argument);
    }

    const auto response = request(message);

    return response.has_value() && response.value() == "register 1";
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

    if (fields.size() < 3 || fields[0] != "access")
    {
      return std::nullopt;
    }

    const auto name = LocalControlProtocol::decode(fields[1]);
    const auto count = LocalControlProtocol::integer(fields[2]);

    if (!name.has_value() || !count.has_value() || count.value() < 0 ||
        fields.size() != static_cast<std::size_t>(count.value()) * 3 + 3)
    {
      return std::nullopt;
    }

    LocalHostAccess access{name.value(), {}};
    access.addresses.reserve(static_cast<std::size_t>(count.value()));

    for (std::size_t index = 3; index < fields.size(); index += 3)
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
