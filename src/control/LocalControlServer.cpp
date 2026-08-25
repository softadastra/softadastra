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
  namespace
  {
    constexpr std::size_t maximum_message_size = 16384;
#if defined(_WIN32)
    std::wstring pipe_name(const std::filesystem::path &path)
    { return L"\\\\.\\pipe\\Softadastra-" + std::to_wstring(std::hash<std::wstring>{}(path.wstring())); }
#endif

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
#if defined(_WIN32)
    if (pipe_ != INVALID_HANDLE_VALUE) return true;
    const auto name=pipe_name(path_);
    pipe_=::CreateNamedPipeW(name.c_str(),PIPE_ACCESS_DUPLEX,PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_NOWAIT,1,maximum_message_size,maximum_message_size,0,nullptr);
    return pipe_ != INVALID_HANDLE_VALUE;
#else
    return false;
#endif
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
        else if (fields.size() == 1 && fields[0] == "shutdown" && shutdown_handler_)
        {
          response = "shutdown 1";
          shutdown_handler_();
        }
        else response = handle(server_, request);
        static_cast<void>(::send(client, response.data(), response.size(), MSG_NOSIGNAL));
      }

      ::close(client);
    }
#else
#if defined(_WIN32)
    if (pipe_ == INVALID_HANDLE_VALUE) return false;
    if (!::ConnectNamedPipe(pipe_,nullptr)) { const auto error=::GetLastError(); if(error==ERROR_PIPE_LISTENING) return false; if(error!=ERROR_PIPE_CONNECTED) return false; }
    std::array<char,maximum_message_size> buffer{}; DWORD received{};
    bool processed=false;
    if(::ReadFile(pipe_,buffer.data(),static_cast<DWORD>(buffer.size()),&received,nullptr)&&received>0) { const std::string_view request(buffer.data(),received); const auto fields=LocalControlProtocol::fields(request); std::string response; if(fields.size()==1&&fields[0]=="shutdown"&&shutdown_handler_){response="shutdown 1";shutdown_handler_();}else response=handle(server_,request); DWORD sent{};static_cast<void>(::WriteFile(pipe_,response.data(),static_cast<DWORD>(response.size()),&sent,nullptr));processed=true; }
    ::FlushFileBuffers(pipe_);::DisconnectNamedPipe(pipe_);return processed;
#else
    return false;
#endif
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
#if defined(_WIN32)
    if (pipe_ != INVALID_HANDLE_VALUE) { ::DisconnectNamedPipe(pipe_); ::CloseHandle(pipe_); pipe_=INVALID_HANDLE_VALUE; }
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

    if (fields[0] == "host-pid" && fields.size() == 1)
    {
#if defined(__linux__)
      return "host-pid " + std::to_string(::getpid());
#else
      return "host-pid -";
#endif
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
                             " " + LocalControlProtocol::encode(access.primary_ipv4) +
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

    if (fields[0] == "network-capability" && fields.size() == 1)
    {
      const NetworkCapability capability = server.network_capability();
      return "network-capability " +
             std::to_string(static_cast<int>(capability.state)) + " " +
             LocalControlProtocol::encode(capability.primary_ipv4) + " " +
             LocalControlProtocol::encode(capability.primary_interface) + " " +
             std::to_string(static_cast<int>(capability.interface_type)) + " " +
             std::to_string(static_cast<int>(capability.local_network_state)) + " " +
             std::to_string(static_cast<int>(capability.managed_network_capability));
    }
    if (fields[0] == "managed-network-status" && fields.size() == 1) { const auto status=server.managed_network_status(); return "managed-network-status "+std::to_string(static_cast<int>(status.capability))+" "+std::to_string(static_cast<int>(status.state))+" "+LocalControlProtocol::encode(status.interface_name)+" "+LocalControlProtocol::encode(status.ipv4)+" "+LocalControlProtocol::encode(status.ssid); }
    if (fields[0] == "managed-network-start" && fields.size() == 1) return "managed-network-start "+std::to_string(static_cast<int>(server.start_managed_network()));
    if (fields[0] == "managed-network-stop" && fields.size() == 1) { const auto status=server.managed_network_status(); return "managed-network-stop "+std::to_string(status.state==ManagedNetworkState::Running&&server.stop_managed_network()?1:0); }

    if (fields[0] == "local-access" && fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      const auto access = id.has_value()
                              ? server.local_access(SoftwareId(id.value()))
                              : std::nullopt;
      if (!access.has_value())
      {
        return "error";
      }

      return "local-access " + std::to_string(static_cast<int>(access->state)) +
             " " + std::to_string(static_cast<int>(access->protocol)) +
             " " + std::to_string(access->port) + " " +
             LocalControlProtocol::encode(access->ipv4) + " " +
             LocalControlProtocol::encode(access->url) + " " +
             std::to_string(static_cast<int>(access->network)) + " " +
             std::to_string(static_cast<int>(access->local_network_state)) + " " +
             std::to_string(static_cast<int>(access->managed_network_capability)) + " " +
             std::to_string(access->managed_network_start_failed ? 1 : 0);
    }

    if (fields[0] == "access-point" && fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      if (!id.has_value())
        return "error";
      const auto access_point = server.access_point(SoftwareId(id.value()));
      if (!access_point.has_value())
        return "access-point 0 - 0";
      return "access-point 1 " + std::string(AccessPoint::name(access_point->protocol())) +
             " " + std::to_string(access_point->port());
    }

    if (fields[0] == "local-gateway-target" && fields.size() == 2)
    {
      const auto host = LocalControlProtocol::decode(fields[1]);
      if (!host) return "error";
      const auto target = server.local_gateway_target(*host);
      if (target.result == LocalGatewayLookup::NotFound) return "not-found";
      if (target.result == LocalGatewayLookup::Unavailable) return "unavailable";
      return "http " + std::to_string(target.port);
    }

    if (fields[0] == "local-reachability" && fields.size() == 1)
      return "local-reachability " + std::to_string(static_cast<int>(server.local_reachability_state()));

    if (fields[0] == "project" && fields.size() == 2)
    {
      const auto identity = LocalControlProtocol::decode(fields[1]);
      if (!identity.has_value()) return "error";
      const auto entry = server.software_by_project_identity(ProjectIdentity(identity.value()));
      return entry.has_value() ? "project 1 " + LocalControlProtocol::encode(entry->id().value()) +
                                     " " + LocalControlProtocol::encode(entry->process_spec().executable())
                               : "project 0 -";
    }

    if (fields[0] == "project-root" && fields.size() == 3)
    {
      const auto identity = LocalControlProtocol::decode(fields[1]);
      const auto root = LocalControlProtocol::decode(fields[2]);
      if (!identity.has_value() || !root.has_value()) return "error";
      return std::string("project-root ") + (server.update_project_root(ProjectIdentity(identity.value()), root.value()) ? "1" : "0");
    }

    if (fields[0] == "software-project" && fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      if (!id.has_value()) return "error";
      const auto state = server.software_state(SoftwareId(id.value()));
      if (!state.has_value()) return "software-project 0 -";
      const auto identity = server.project_identity(SoftwareId(id.value()));
      return "software-project 1 " + LocalControlProtocol::encode(identity.has_value() ? identity->value() : "");
    }

    if (fields[0] == "software" && fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      if (!id.has_value()) return "error";
      const auto entry = server.software(SoftwareId(id.value()));
      return entry.has_value() ? "software 1 " + LocalControlProtocol::encode(entry->process_spec().executable()) : "software 0 -";
    }

    if (fields[0] == "software-list" && fields.size() == 1)
    {
      const auto entries = server.software();
      std::string response = "software-list " + std::to_string(entries.size());
      for (const auto &entry : entries)
      {
        const auto access = entry.access_point();
        response += " " + LocalControlProtocol::encode(entry.id().value()) +
                    " " + LocalControlProtocol::encode(entry.name()) +
                    " " + std::to_string(static_cast<int>(entry.state())) +
                    " " + LocalControlProtocol::encode(entry.process_spec().executable()) +
                    " " + LocalControlProtocol::encode(entry.process_spec().working_directory().value_or("")) +
                    " " + LocalControlProtocol::encode(entry.project_identity().has_value() ? entry.project_identity()->value() : "") +
                    " " + LocalControlProtocol::encode(entry.declared_command()) +
                    " " + std::to_string(entry.pid().value_or(-1)) +
                    " " + (access ? std::string(AccessPoint::name(access->protocol())) : "-") +
                    " " + std::to_string(access ? access->port() : 0);
      }
      return response;
    }

    if (fields[0] == "remove" && fields.size() == 2)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      return id ? std::string("remove ") + (server.remove_software(SoftwareId(*id)) ? "1" : "0") : "error";
    }

    if (fields[0] == "link-project" && fields.size() == 4)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      const auto identity = LocalControlProtocol::decode(fields[2]);
      const auto root = LocalControlProtocol::decode(fields[3]);
      if (!id.has_value() || !identity.has_value() || !root.has_value() || identity->empty()) return "error";
      return std::string("link-project ") + (server.link_project(SoftwareId(id.value()), ProjectIdentity(identity.value()), root.value()) ? "1" : "0");
    }

    if (fields[0] == "sync" && fields.size() >= 8)
    {
      const auto id = LocalControlProtocol::decode(fields[1]); const auto name = LocalControlProtocol::decode(fields[2]); const auto executable = LocalControlProtocol::decode(fields[3]); const auto root = LocalControlProtocol::decode(fields[4]);
      const auto protocol = fields[5] == "-" ? std::optional<AccessProtocol>{} : AccessPoint::protocol(fields[5]);
      const auto port = LocalControlProtocol::integer(fields[6]); const auto count = LocalControlProtocol::integer(fields[7]);
      if (!id || !name || name->empty() || !executable || !root || !port || !count || count.value() < 0 || fields.size() != static_cast<std::size_t>(count.value()) + 8 || (fields[5] == "-" && port.value() != 0) || (fields[5] != "-" && (!protocol || port.value() < 1 || port.value() > 65535))) return "error";
      std::vector<std::string> arguments;
      for (std::size_t index = 8; index < fields.size(); ++index) { const auto argument = LocalControlProtocol::decode(fields[index]); if (!argument) return "error"; arguments.push_back(argument.value()); }
      const auto access = protocol ? AccessPoint::create(protocol.value(), static_cast<std::uint16_t>(port.value())) : std::nullopt;
      return std::string("sync ") + (server.synchronize_software(SoftwareId(id.value()), ProcessSpec(executable.value(), std::move(arguments), root->empty() ? std::nullopt : std::optional<std::string>(root.value())), access, name.value()) ? "1" : "0");
    }

    if (fields[0] == "register" && fields.size() >= 9)
    {
      const auto id = LocalControlProtocol::decode(fields[1]);
      const auto name = LocalControlProtocol::decode(fields[2]);
      const auto executable = LocalControlProtocol::decode(fields[3]);
      const auto project_identity = LocalControlProtocol::decode(fields[4]);
      const auto working_directory = LocalControlProtocol::decode(fields[5]);
      const auto protocol = fields[6] == "-" ? std::optional<AccessProtocol>{} : AccessPoint::protocol(fields[6]);
      const auto port = LocalControlProtocol::integer(fields[7]); const auto count = LocalControlProtocol::integer(fields[8]);

      if (!id.has_value() || !executable.has_value() || !project_identity.has_value() || !working_directory.has_value() || !count.has_value() || !port.has_value() ||
          !name.has_value() || name->empty() || count.value() < 0 || fields.size() != static_cast<std::size_t>(count.value()) + 9 ||
          (fields[6] == "-" && port.value() != 0) || (fields[6] != "-" && (!protocol.has_value() || port.value() < 1 || port.value() > 65535)))
      {
        return "error";
      }

      std::vector<std::string> arguments;
      arguments.reserve(static_cast<std::size_t>(count.value()));

      for (std::size_t index = 9; index < fields.size(); ++index)
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
                  ProcessSpec(executable.value(), std::move(arguments), working_directory.value().empty() ? std::nullopt : std::optional<std::string>(working_directory.value())),
                  protocol.has_value()
                      ? AccessPoint::create(protocol.value(), static_cast<std::uint16_t>(port.value()))
                      : std::nullopt,
                  project_identity.value().empty() ? std::nullopt : std::optional<ProjectIdentity>(ProjectIdentity(project_identity.value())), name.value())
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
