/**
 *
 *  @file Cli.cpp
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

#include "cli/Cli.hpp"

#include "cli/AccessUrl.hpp"
#include "platform/QrCode.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
  const char *software_state_name(softadastra::SoftwareState state) noexcept
  {
    switch (state)
    {
    case softadastra::SoftwareState::Stopped:
      return "stopped";

    case softadastra::SoftwareState::Starting:
      return "starting";

    case softadastra::SoftwareState::Running:
      return "running";

    case softadastra::SoftwareState::Failed:
      return "failed";
    }

    return "unknown";
  }

  void print_operation_error(
      const softadastra::SoftwareOperationResult &result)
  {
    const auto error = result.error();

    if (!error.has_value())
    {
      return;
    }

    switch (error.value())
    {
    case softadastra::SoftwareOperationError::SoftwareUnknown:
      std::cerr << "software is unknown";
      return;

    case softadastra::SoftwareOperationError::AlreadyRunning:
      std::cerr << "software is already running";
      return;

    case softadastra::SoftwareOperationError::NotRunning:
      std::cerr << "software is not running";
      return;

    case softadastra::SoftwareOperationError::ExecutableNotFound:
      std::cerr << "executable not found";
      return;

    case softadastra::SoftwareOperationError::PermissionDenied:
      std::cerr << "permission denied";
      return;

    case softadastra::SoftwareOperationError::LaunchFailed:
      std::cerr << "launch failed";
      return;

    case softadastra::SoftwareOperationError::ProcessExitedSuccessfully:
      std::cerr << "process exited successfully";
      return;

    case softadastra::SoftwareOperationError::ProcessExitedWithNonZeroCode:
      std::cerr << "process exited with code "
                << result.exit_code().value_or(-1);
      return;

    case softadastra::SoftwareOperationError::StopFailed:
      std::cerr << "stop failed";
      return;
    }
  }

  void print_usage()
  {
    std::cout
        << "Usage:\n"
        << "  softadastra register <software-id> <executable> [arguments...]\n"
        << "  softadastra start <software-id>\n"
        << "  softadastra stop <software-id>\n"
        << "  softadastra restart <software-id>\n"
        << "  softadastra status <software-id>\n"
        << "  softadastra status\n"
        << "  softadastra connectivity\n"
        << "  softadastra access [port]\n"
        << "  softadastra remote enable <ipv4-address> <port>\n"
        << "  softadastra remote disable\n"
        << "  softadastra help\n";
  }

} // namespace

namespace softadastra
{

  Cli::Cli(ControlClient &client) noexcept
      : client_(client)
  {
  }

  int Cli::run(int argc, const char *const argv[])
  {
    if (argc < 2)
    {
      print_usage();
      return 2;
    }

    const std::string command(argv[1]);

    if (command == "help" ||
        command == "--help" ||
        command == "-h")
    {
      print_usage();
      return 0;
    }

    if (!client_.host_available())
    {
      std::cerr << "Host is not running\n";
      return 1;
    }

    if (command == "remote")
    {
      if ((argc != 3 || std::string(argv[2]) != "disable") &&
          (argc != 5 || std::string(argv[2]) != "enable"))
      {
        std::cerr << "remote requires enable <ipv4-address> <port> or disable\n";
        return 2;
      }
      const std::string request = argc == 3 ? "remote disable" :
          "remote enable " + std::string(argv[3]) + " " + std::string(argv[4]);
      const auto response = client_.request(request);
      if (!response.has_value() || response.value() == "error")
      {
        std::cerr << "failed to update remote access\n";
        return 1;
      }
      std::cout << (response.value() == "remote 1" ? "remote access: enabled\n" : "remote access: disabled\n");
      return 0;
    }

    if (command == "connectivity")
    {
      if (argc != 2)
      {
        std::cerr << "connectivity does not accept arguments\n";
        return 2;
      }

      if (!client_.connectivity_available())
      {
        std::cout << "network: unavailable\n";
        return 0;
      }

      std::cout << "network: available\n"
                << "connected: "
                << (client_.connected() ? "yes" : "no")
                << '\n';

      return 0;
    }

    if (command == "access")
    {
      if (argc != 2 && argc != 3)
      {
        std::cerr << "access accepts an optional port from 1 to 65535\n";
        return 2;
      }

      std::optional<std::uint16_t> port;

      if (argc == 3)
      {
        port = AccessUrl::port(argv[2]);

        if (!port.has_value())
        {
          std::cerr << "port must be between 1 and 65535\n";
          return 2;
        }
      }

      const auto access = client_.local_access();

      if (!access.has_value())
      {
        std::cerr << "Host access information is unavailable\n";
        return 1;
      }

      if (port.has_value())
      {
        if (access->primary_ipv4.empty())
        {
          std::cerr << "current IPv4 address is unavailable\n";
          return 1;
        }

        const std::string url = AccessUrl::http(
            access->primary_ipv4,
            port.value());

        std::cout << "hostname: "
                  << (access->host_name.empty() ? "unavailable" : access->host_name)
                  << "\nipv4: " << access->primary_ipv4
                  << "\nurl: " << url << "\n\n";

        if (!QrCode::print(url))
        {
          std::cout << "QR generation is unavailable.\n";
        }

        std::cout << "\nScan with your phone.\n";
        return 0;
      }

      if (!access->host_name.empty())
      {
        std::cout << "hostname: " << access->host_name << '\n';
      }

      for (const auto &address : access->addresses)
      {
        std::cout << (address.family == LocalAddressFamily::IPv4
                          ? "ipv4"
                          : "ipv6")
                  << " " << address.interface_name
                  << ": " << address.value << '\n';
      }

      if (access->host_name.empty() && access->addresses.empty())
      {
        std::cout << "no active non-loopback address\n";
      }

      std::cout << "hosted software endpoints are managed by the software\n";
      return 0;
    }

    if (command == "status" && argc == 2)
    {
      std::cout << "Host: running\n";
      return 0;
    }

    if (command == "register")
    {
      if (argc < 4)
      {
        std::cerr
            << "register requires a software identifier and an executable\n";
        return 2;
      }

      std::vector<std::string> arguments;
      arguments.reserve(static_cast<std::size_t>(argc - 4));

      for (int index = 4; index < argc; ++index)
      {
        arguments.emplace_back(argv[index]);
      }

      const SoftwareId id(argv[2]);
      const ProcessSpec process_spec(
          argv[3],
          std::move(arguments));

      if (!client_.register_software(id, process_spec))
      {
        std::cerr
            << "failed to register software: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << "registered: "
          << id.value()
          << '\n';

      return 0;
    }

    if (argc != 3)
    {
      std::cerr
          << command
          << " requires a software identifier\n";

      return 2;
    }

    const SoftwareId id(argv[2]);

    if (command == "start")
    {
      const SoftwareOperationResult result =
          client_.start_software(id);

      if (!result)
      {
        std::cerr
            << "failed to start "
            << id.value()
            << ": ";
        print_operation_error(result);
        std::cerr
            << '\n';

        return 1;
      }

      std::cout
          << "started: "
          << id.value()
          << '\n';

      return 0;
    }

    if (command == "stop")
    {
      const SoftwareOperationResult result =
          client_.stop_software(id);

      if (!result)
      {
        std::cerr
            << "cannot stop "
            << id.value()
            << ": ";
        print_operation_error(result);
        std::cerr
            << '\n';

        return 1;
      }

      std::cout
          << "stopped: "
          << id.value()
          << '\n';

      return 0;
    }

    if (command == "restart")
    {
      const SoftwareOperationResult result =
          client_.restart_software(id);

      if (!result)
      {
        std::cerr
            << "failed to restart "
            << id.value()
            << ": ";
        print_operation_error(result);
        std::cerr << '\n';
        return 1;
      }

      std::cout
          << "restarted: "
          << id.value()
          << '\n';
      return 0;
    }

    if (command == "status")
    {
      const auto state = client_.software_state(id);

      if (!state.has_value())
      {
        std::cerr
            << "software not found: "
            << id.value()
            << '\n';

        return 1;
      }

      const auto result = client_.software_result(id);

      if (result.has_value() && !result.value())
      {
        std::cout
            << id.value()
            << " "
            << software_state_name(state.value())
            << ": ";
        const auto error = result->error();

        switch (error.value())
        {
        case SoftwareOperationError::ProcessExitedWithNonZeroCode:
          std::cout << "process exited with code "
                    << result->exit_code().value_or(-1);
          break;

        case SoftwareOperationError::ProcessExitedSuccessfully:
          std::cout << "process exited successfully";
          break;

        default:
          std::cout << software_state_name(state.value());
          break;
        }

        std::cout << '\n';
        return 0;
      }

      std::cout
          << id.value()
          << ": "
          << software_state_name(state.value())
          << '\n';

      return 0;
    }

    std::cerr
        << "unknown command: "
        << command
        << '\n';

    print_usage();

    return 2;
  }

} // namespace softadastra
