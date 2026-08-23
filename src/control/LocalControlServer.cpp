/**
 *
 *  @file LocalControlServer.cpp
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

#include "control/LocalControlServer.hpp"

#include "control/LocalControlProtocol.hpp"
#include "control/RemoteAccessConfig.hpp"
#include "control/RemoteControlServer.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <utility>

#if defined(__linux__)

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#endif

namespace softadastra
{
  namespace
  {
    constexpr std::size_t maximum_message_size = 16384;

    std::string operation_response(const SoftwareOperationResult &result)
    {
      const auto error = result.error();
      const auto code = result.exit_code();
      return "operation " + std::to_string(error.has_value()
                                                ? static_cast<int>(error.value())
                                                : -1) +
             " " + std::to_string(code.has_value() ? 1 : 0) +
             " " + std::to_string(code.value_or(0));
    }

  } // namespace

  LocalControlServer::LocalControlServer(
      ControlServer &server,
      std::filesystem::path path,
      RemoteAccessConfig *remote_config,
      RemoteControlServer *remote_server) noexcept
      : server_(server),
        path_(std::move(path)),
        remote_config_(remote_config),
        remote_server_(remote_server)
  {
  }

  LocalControlServer::~LocalControlServer()
  {
    stop();
  }

  bool LocalControlServer::start()
  {
#if defined(__linux__)
    if (descriptor_ >= 0)
    {
      return true;
    }

    if (path_.string().size() >= sizeof(sockaddr_un::sun_path))
    {
      return false;
    }

    std::error_code error;
    std::filesystem::create_directories(path_.parent_path(), error);

    if (error)
    {
      return false;
    }

    descriptor_ = ::socket(AF_UNIX, SOCK_SEQPACKET | SOCK_NONBLOCK, 0);

    if (descriptor_ < 0)
    {
      return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, path_.c_str(), sizeof(address.sun_path) - 1);
    ::unlink(address.sun_path);

    if (::bind(descriptor_, reinterpret_cast<const sockaddr *>(&address), sizeof(address)) != 0 ||
        ::listen(descriptor_, 16) != 0)
    {
      stop();
      return false;
    }

    return true;
#else
    return false;
#endif
  }

  bool LocalControlServer::process_pending()
  {
#if defined(__linux__)
    if (descriptor_ < 0)
    {
      return false;
    }

    bool processed = false;

    for (;;)
    {
      const int client = ::accept4(descriptor_, nullptr, nullptr, SOCK_CLOEXEC);

      if (client < 0)
      {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
          return processed;
        }

        return processed;
      }

      processed = true;

      std::array<char, maximum_message_size> buffer{};
      const ssize_t received = ::recv(client, buffer.data(), buffer.size(), 0);

      if (received > 0)
      {
        const std::string_view request(buffer.data(), static_cast<std::size_t>(received));
        std::string response;
        const auto fields = LocalControlProtocol::fields(request);
        if (remote_config_ != nullptr && remote_server_ != nullptr && fields.size() >= 2 && fields[0] == "remote")
        {
          RemoteAccessSettings settings;
          if (fields[1] == "disable" && fields.size() == 2)
          {
            response = remote_config_->save(settings) && remote_server_->apply() ? "remote 0" : "error";
          }
          else if (fields[1] == "enable" && fields.size() == 4)
          {
            const auto port = LocalControlProtocol::integer(fields[3]);
            const bool valid_port = port.has_value() && port.value() > 0 && port.value() <= 65535;
            settings = {true, fields[2], valid_port ? static_cast<std::uint16_t>(port.value()) : static_cast<std::uint16_t>(0)};
            response = remote_config_->save(settings) && remote_server_->apply() ? "remote 1" : "error";
          }
          else response = "error";
        }
        else response = handle(server_, request);
        static_cast<void>(::send(client, response.data(), response.size(), MSG_NOSIGNAL));
      }

      ::close(client);
    }
#else
    return false;
#endif
  }

  void LocalControlServer::stop() noexcept
  {
#if defined(__linux__)
    if (descriptor_ >= 0)
    {
      ::close(descriptor_);
      descriptor_ = -1;
    }

    std::error_code error;
    std::filesystem::remove(path_, error);
#endif
  }

  std::string LocalControlServer::handle(ControlServer &server, std::string_view request)
  {
    const auto fields = LocalControlProtocol::fields(request);

    if (fields.empty())
    {
      return "error";
    }

    if (fields[0] == "ping" && fields.size() == 1)
    {
      return "ok";
    }

    if (fields[0] == "connectivity" && fields.size() == 1)
    {
      return "connectivity " +
             std::to_string(server.connectivity_available() ? 1 : 0) +
             " " + std::to_string(server.connected() ? 1 : 0);
    }

    if (fields[0] == "access" && fields.size() == 1)
    {
      const LocalHostAccess access = server.local_access();
      std::string response = "access " +
                             LocalControlProtocol::encode(access.host_name) +
                             " " + std::to_string(access.addresses.size());

      for (const auto &address : access.addresses)
      {
        response += " ";
        response += address.family == LocalAddressFamily::IPv4 ? "4" : "6";
        response += " ";
        response += LocalControlProtocol::encode(address.interface_name);
        response += " ";
        response += LocalControlProtocol::encode(address.value);
      }

      return response;
    }

    if (fields[0] == "register" && fields.size() >= 4)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      const auto executable = LocalControlProtocol::decode(fields[2]);
      const auto count = LocalControlProtocol::integer(fields[3]);

      if (!id.has_value() || !executable.has_value() || !count.has_value() ||
          count.value() < 0 || fields.size() != static_cast<std::size_t>(count.value()) + 4)
      {
        return "error";
      }

      std::vector<std::string> arguments;
      arguments.reserve(static_cast<std::size_t>(count.value()));

      for (std::size_t index = 4; index < fields.size(); ++index)
      {
        const auto argument = LocalControlProtocol::decode(fields[index]);

        if (!argument.has_value())
        {
          return "error";
        }

        arguments.push_back(argument.value());
      }

      return std::string("register ") +
             (server.register_software(
                  SoftwareId(id.value()),
                  ProcessSpec(executable.value(), std::move(arguments)))
                  ? "1"
                  : "0");
    }

    if ((fields[0] == "start" || fields[0] == "stop" ||
         fields[0] == "restart" || fields[0] == "status") &&
        fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);

      if (!id.has_value())
      {
        return "error";
      }

      const SoftwareId software_id(id.value());

      if (fields[0] == "start")
      {
        return operation_response(server.start_software(software_id));
      }

      if (fields[0] == "stop")
      {
        return operation_response(server.stop_software(software_id));
      }

      if (fields[0] == "restart")
      {
        return operation_response(server.restart_software(software_id));
      }

      const auto state = server.software_state(software_id);
      const auto result = server.software_result(software_id);
      const auto error = result.has_value() ? result->error() : std::nullopt;
      const auto code = result.has_value() ? result->exit_code() : std::nullopt;
      return "status " + std::to_string(state.has_value()
                                              ? static_cast<int>(state.value())
                                              : -1) +
             " " + std::to_string(error.has_value()
                                        ? static_cast<int>(error.value())
                                        : -1) +
             " " + std::to_string(code.has_value() ? 1 : 0) +
             " " + std::to_string(code.value_or(0));
    }

    return "error";
  }

} // namespace softadastra
