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
#include "platform/NativeDataDirectory.hpp"
#include "platform/ProcessSpec.hpp"
#include "platform/QrCode.hpp"
#include "software/AccessPoint.hpp"
#include "software/ProjectConfig.hpp"
#include "software/ProjectIdentity.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

namespace
{
  using namespace softadastra;

  const char *state_name(
      SoftwareState state) noexcept
  {
    switch (state)
    {
    case SoftwareState::Stopped:
      return "stopped";

    case SoftwareState::Starting:
      return "starting";

    case SoftwareState::Running:
      return "running";

    case SoftwareState::Failed:
      return "failed";
    }

    return "unknown";
  }

  std::string access_name(
      const std::optional<AccessPoint> &access)
  {
    return access
               ? std::string(
                     AccessPoint::name(access->protocol())) +
                     ":" +
                     std::to_string(access->port())
               : "-";
  }

  std::string command_name(
      const SoftwareEntry &entry)
  {
    if (!entry.declared_command().empty())
    {
      return entry.declared_command();
    }

    const auto &spec =
        entry.process_spec();

    if (spec.executable() == "/bin/sh" &&
        spec.arguments().size() == 2 &&
        spec.arguments()[0] == "-lc")
    {
      return spec.arguments()[1];
    }

    std::string value =
        spec.executable();

    for (const auto &argument : spec.arguments())
    {
      value += " " + argument;
    }

    return value;
  }

  void operation_error(
      const SoftwareOperationResult &result)
  {
    switch (
        result.error().value_or(
            SoftwareOperationError::LaunchFailed))
    {
    case SoftwareOperationError::SoftwareUnknown:
      std::cerr << "software is unknown";
      break;

    case SoftwareOperationError::AlreadyRunning:
      std::cerr << "software is already running";
      break;

    case SoftwareOperationError::NotRunning:
      std::cerr << "software is not running";
      break;

    case SoftwareOperationError::ExecutableNotFound:
      std::cerr << "command could not be started";
      break;

    case SoftwareOperationError::PermissionDenied:
      std::cerr << "permission denied";
      break;

    case SoftwareOperationError::LaunchFailed:
      std::cerr << "launch failed";
      break;

    case SoftwareOperationError::ProcessExitedSuccessfully:
      std::cerr << "process exited successfully";
      break;

    case SoftwareOperationError::ProcessExitedWithNonZeroCode:
      std::cerr
          << "process exited with code "
          << result.exit_code().value_or(-1);
      break;

    case SoftwareOperationError::StopFailed:
      std::cerr << "stop failed";
      break;
    }
  }

  void unknown_software(
      const std::string &name)
  {
    std::cerr
        << "Software not found: "
        << name
        << "\n\nView registered software with:\n\n"
        << "  softadastra list\n";
  }

  void no_project(
      const std::string &command)
  {
    if (command == "logs" ||
        command == "remove")
    {
      std::cerr
          << "No Softadastra project found.\n\n"
          << "Select a registered software explicitly:\n\n"
          << "  softadastra "
          << command
          << " <name>\n";

      return;
    }

    std::cerr
        << "No Softadastra project found.\n\n"
        << "Initialize the current project with:\n\n"
        << "  softadastra init\n\n"
        << "Or select a registered software explicitly:\n\n"
        << "  softadastra "
        << command
        << " <name>\n";
  }

  void usage()
  {
    std::cout
        << "Softadastra runs software on this Host.\n\n"
        << "Usage:\n"
        << "  softadastra <command> [arguments]\n\n"
        << "Project:\n"
        << "  init [name] [--command <command>] [--access http:port]\n"
        << "  run [name]\n\n"
        << "Software:\n"
        << "  start [name]\n"
        << "  stop [name]\n"
        << "  restart [name]\n"
        << "  status [name]\n"
        << "  info [name]\n"
        << "  access [name]\n"
        << "  logs [name] [--follow]\n\n"
        << "Inventory:\n"
        << "  list [--running|--stopped]\n\n"
        << "Host:\n"
        << "  connectivity\n"
        << "  network info\n"
        << "  remote enable <ipv4-address> <port>\n"
        << "  remote disable\n\n"
        << "Advanced:\n"
        << "  register <name> [--access http:port] -- <command> "
           "[arguments...]\n";
  }

  void command_usage(
      const std::string &command)
  {
    if (command == "logs")
    {
      std::cout
          << "Usage:\n"
          << "  softadastra logs [software-name] [--follow]\n\n"
          << "Without a name:\n"
          << "  Show logs for the current Softadastra project.\n\n"
          << "With a name:\n"
          << "  Show logs for a registered software.\n\n"
          << "Options:\n"
          << "  -f, --follow    Follow new log output\n";
    }
    else if (command == "access")
    {
      std::cout
          << "Usage:\n"
          << "  softadastra access [software-name]\n\n"
          << "Show local access for a Software. If no local network is "
             "available, Softadastra may start a safe managed local network "
             "when the Host supports it.\n";
    }
    else if (command == "network")
    {
      std::cout
          << "Usage:\n"
          << "  softadastra network info\n"
          << "  softadastra network status\n"
          << "  softadastra network start\n"
          << "  softadastra network stop\n\n"
          << "Commands:\n"
          << "  info      Show current Host network state and capabilities\n"
          << "  status    Show managed local network state\n"
          << "  start     Start the managed local network\n"
          << "  stop      Stop the managed local network\n";
    }
    else if (command == "init")
    {
      std::cout
          << "Usage: softadastra init [name] [--command <command>] "
             "[--access http:port]\n";
    }
    else if (command == "run")
    {
      std::cout
          << "Usage: softadastra run [name]\n";
    }
    else if (command == "list")
    {
      std::cout
          << "Usage: softadastra list [--running|--stopped]\n";
    }
    else if (command == "register")
    {
      std::cout
          << "Usage: softadastra register <name> [--access http:port] -- "
             "<command> [arguments...]\n";
    }
    else if (command == "connectivity")
    {
      std::cout
          << "Usage: softadastra connectivity\n";
    }
    else if (command == "remote")
    {
      std::cout
          << "Usage:\n"
          << "  softadastra remote enable <ipv4-address> <port>\n"
          << "       softadastra remote disable\n";
    }
    else if (command == "start" ||
             command == "stop" ||
             command == "restart" ||
             command == "status" ||
             command == "info")
    {
      std::cout
          << "Usage: softadastra "
          << command
          << " [name]\n";
    }
    else
    {
      std::cerr
          << "Unknown command: "
          << command
          << '\n';

      usage();
    }
  }

  bool is_help(
      const std::string &value)
  {
    return value == "-h" ||
           value == "--help";
  }

  std::optional<AccessPoint> parse_access(
      const std::string &value)
  {
    const auto position =
        value.find(':');

    if (position == std::string::npos)
    {
      return std::nullopt;
    }

    const auto protocol =
        AccessPoint::protocol(
            value.substr(0, position));

    const auto port =
        AccessUrl::port(
            value.substr(position + 1));

    return protocol && port
               ? AccessPoint::create(
                     *protocol,
                     *port)
               : std::nullopt;
  }

  ProcessSpec shell_process(
      const std::string &command,
      const std::string &working_directory)
  {
#if defined(_WIN32)

    return ProcessSpec(
        "cmd.exe",
        {"/C", command},
        working_directory);

#else

    return ProcessSpec(
        "/bin/sh",
        {"-lc", command},
        working_directory);

#endif
  }

  struct Target
  {
    SoftwareId id{""};
    std::string name;
    std::optional<SoftwareEntry> entry;
    std::optional<std::filesystem::path> root;
    std::optional<ProjectConfig> config;
  };

  std::optional<Target> resolve_target(
      ControlClient &client,
      const std::optional<std::string> &name,
      const std::string &command)
  {
    if (name)
    {
      Target target{
          SoftwareId(*name),
          *name,
          client.software(
              SoftwareId(*name)),
          std::nullopt,
          std::nullopt};

      if (!target.entry)
      {
        unknown_software(*name);
        return std::nullopt;
      }

      return target;
    }

    std::string error;

    const auto config =
        ProjectConfigFile::find(
            std::filesystem::current_path(),
            &error);

    if (!error.empty())
    {
      std::cerr
          << error
          << '\n';

      return std::nullopt;
    }

    if (config)
    {
      Target target{
          SoftwareId(config->second.id.value()),
          config->second.name,
          client.software(
              SoftwareId(
                  config->second.id.value())),
          config->first,
          config->second};

      if (!target.entry)
      {
        unknown_software(
            config->second.name);

        return std::nullopt;
      }

      return target;
    }

    const auto legacy =
        ProjectIdentity::find(
            std::filesystem::current_path());

    if (!legacy)
    {
      no_project(command);
      return std::nullopt;
    }

    const auto entry =
        client.software_by_project_identity(
            legacy->second);

    if (!entry)
    {
      std::cerr
          << "This Softadastra project is not registered on this Host.\n";

      return std::nullopt;
    }

    return Target{
        entry->id(),
        entry->id().value(),
        entry,
        legacy->first,
        std::nullopt};
  }

  bool sync_project(
      ControlClient &client,
      Target &target)
  {
    if (!target.entry)
    {
      return true;
    }

    if (!target.config)
    {
      if (target.root &&
          target.entry->project_identity() &&
          !client.update_project_root(
              *target.entry->project_identity(),
              target.root->string()))
      {
        std::cerr
            << "failed to update project location on this Host\n";

        return false;
      }

      target.entry =
          client.software(target.id);

      return true;
    }

    if (!target.root)
    {
      return true;
    }

    const auto existing =
        target.entry->project_identity();

    const auto legacy =
        ProjectIdentity::find(
            *target.root);

    if (existing &&
        existing->value() != target.config->id.value() &&
        (!legacy ||
         legacy->second != *existing))
    {
      std::cerr
          << "Software identifier is linked to another project: "
          << target.id.value()
          << '\n';

      return false;
    }

    const ProcessSpec configured_process =
        shell_process(
            target.config->command,
            target.root->string());

    auto access_points =
        target.config->access_points;

    if (access_points.empty() &&
        target.config->access)
    {
      access_points.push_back(
          *target.config->access);
    }

    const bool configuration_changed =
        target.entry->name() != target.config->name ||
        target.entry->process_spec().executable() !=
            configured_process.executable() ||
        target.entry->process_spec().arguments() !=
            configured_process.arguments() ||
        target.entry->process_spec().working_directory() !=
            configured_process.working_directory() ||
        target.entry->access_points() !=
            access_points;

    if (configuration_changed &&
        !client.synchronize_software(
            target.id,
            configured_process,
            std::move(access_points),
            target.config->name))
    {
      std::cerr
          << "Stop the application before changing its configuration.\n";

      return false;
    }

    if (existing &&
        !client.update_project_root(
            *existing,
            target.root->string()))
    {
      std::cerr
          << "failed to update project location on this Host\n";

      return false;
    }

    target.entry =
        client.software(target.id);

    return true;
  }

  bool print_access(
      ControlClient &client,
      const Target &target)
  {
    const auto configured =
        target.config
            ? target.config->access
            : target.entry->access_point();

    if (!configured)
    {
      std::cerr
          << "No access configured for: "
          << target.name
          << '\n';

      if (target.root &&
          target.config)
      {
        std::cerr
            << "\nConfigure `access` in:\n\n"
            << "  "
            << (*target.root / "softadastra.toml").string()
            << "\n\nExample:\n\n"
            << "  access = \"http:8080\"\n";
      }
      else
      {
        std::cerr
            << "\nThis global Software has no AccessPoint configured.\n";
      }

      return false;
    }

    const auto access =
        client.local_access(target.id);

    if (!access)
    {
      std::cerr
          << "Local access information is unavailable\n";

      return false;
    }

    const auto entry =
        client.software(target.id);

    if (!entry)
    {
      unknown_software(target.name);
      return false;
    }

    std::cout
        << "Software:      "
        << target.name
        << "\nState:         "
        << state_name(entry->state())
        << "\nAccess:        "
        << access_name(configured)
        << '\n';

    if (access->state == LocalAccessState::Available)
    {
      std::cout
          << "Network:       "
          << local_access_network_name(access->network)
          << "\nLocal URL:     "
          << access->url
          << "\n\n";

      if (!QrCode::print(access->url))
      {
        std::cout
            << "QR generation is unavailable.\n";
      }

      std::cout
          << "\nScan with your phone.\n";

      return true;
    }

    std::cout
        << "Local access:  unavailable\n";

    if (entry->state() == SoftwareState::Stopped)
    {
      std::cout
          << "\nStart it with:\n\n"
          << "  softadastra run"
          << (target.root
                  ? ""
                  : " " + target.name)
          << "\n";
    }
    else if (entry->state() == SoftwareState::Failed)
    {
      std::cout
          << "\nInspect logs with:\n\n"
          << "  softadastra logs"
          << (target.root
                  ? ""
                  : " " + target.name)
          << "\n";
    }
    else if (access->managed_network_start_failed)
    {
      std::cout
          << "\nUnable to start a local Softadastra network.\n";
    }
    else if (
        access->local_network_state ==
        LocalNetworkState::Unavailable)
    {
      std::cout
          << "\nNo local network is available on this Host.\n\n"
          << "Managed network: "
          << managed_network_capability_name(
                 access->managed_network_capability)
          << "\n";
    }

    return false;
  }

} // namespace

namespace softadastra
{
  Cli::Cli(
      ControlClient &client) noexcept
      : client_(client)
  {
  }

  Cli::Cli(
      ControlClient &client,
      const Network &network) noexcept
      : client_(client),
        network_(&network)
  {
  }

  Cli::Cli(
      ControlClient &client,
      const Network &network,
      ManagedNetwork &managed_network) noexcept
      : client_(client),
        network_(&network),
        managed_network_(&managed_network)
  {
  }

  int Cli::run(
      int argc,
      const char *const argv[])
  {
    if (argc < 2)
    {
      usage();
      return 2;
    }

    const std::string command(argv[1]);

    if (command == "help")
    {
      if (argc == 2)
      {
        usage();
      }
      else if (
          argc == 3 &&
          std::string(argv[2]) == "remove")
      {
        std::cout
            << "Usage:\n"
            << "  softadastra remove [software-name]\n\n"
            << "The project files and logs are not deleted.\n";
      }
      else if (
          argc == 3 &&
          std::string(argv[2]) == "logs")
      {
        std::cout
            << "Usage:\n"
            << "  softadastra logs [software-name] [--follow|--clear]\n\n"
            << "Options:\n"
            << "  -f, --follow    Follow new log output\n"
            << "  --clear         Clear stored logs\n";
      }
      else if (argc == 3)
      {
        command_usage(argv[2]);
      }
      else
      {
        std::cerr
            << "help accepts one command\n";

        return 2;
      }

      return 0;
    }

    if (is_help(command))
    {
      usage();
      return 0;
    }

    if (argc >= 3 &&
        is_help(argv[2]))
    {
      if (argc != 3)
      {
        command_usage(command);
        return 2;
      }

      if (command == "remove")
      {
        std::cout
            << "Usage:\n"
            << "  softadastra remove [software-name]\n\n"
            << "The project files and logs are not deleted.\n";
      }
      else if (command == "logs")
      {
        std::cout
            << "Usage:\n"
            << "  softadastra logs [software-name] [--follow|--clear]\n\n"
            << "Options:\n"
            << "  -f, --follow    Follow new log output\n"
            << "  --clear         Clear stored logs\n";
      }
      else
      {
        command_usage(command);
      }

      return 0;
    }

    if (command == "network")
    {
      if (argc != 3)
      {
        command_usage(command);
        return 2;
      }

      const std::string action(argv[2]);

      const auto status =
          managed_network_
              ? std::optional<ManagedNetworkStatus>(
                    managed_network_->status())
              : client_.managed_network_status();

      if (action == "start")
      {
        const auto result =
            managed_network_
                ? std::optional<ManagedNetworkStartResult>(
                      managed_network_->start())
                : client_.start_managed_network();

        if (!result ||
            *result ==
                ManagedNetworkStartResult::Unavailable)
        {
          std::cerr
              << "Managed network is unavailable on this Host.\n";

          return 1;
        }

        if (*result ==
            ManagedNetworkStartResult::WouldDisruptConnection)
        {
          std::cerr
              << "Managed network cannot be started without disrupting "
                 "the current network connection.\n";

          return 1;
        }

        if (*result ==
            ManagedNetworkStartResult::Failed)
        {
          std::cerr
              << "Failed to start Softadastra local network.\n";

          return 1;
        }

        const auto current =
            managed_network_
                ? std::optional<ManagedNetworkStatus>(
                      managed_network_->status())
                : client_.managed_network_status();

        if (!current)
        {
          return 1;
        }

        std::cout
            << (*result ==
                        ManagedNetworkStartResult::AlreadyRunning
                    ? "Softadastra local network is already running.\n"
                    : "Softadastra local network started.\n")
            << "\nNetwork:   "
            << current->ssid
            << "\nIPv4:      "
            << current->ipv4
            << '\n';

        return 0;
      }

      if (action == "stop")
      {
        const auto stopped =
            managed_network_
                ? std::optional<bool>(
                      managed_network_->status().state ==
                          ManagedNetworkState::Running &&
                      managed_network_->stop())
                : client_.stop_managed_network();

        if (!stopped ||
            !*stopped)
        {
          std::cout
              << "Softadastra local network is not running.\n";

          return 0;
        }

        std::cout
            << "Softadastra local network stopped.\n";

        return 0;
      }

      if (action == "status")
      {
        if (!status)
        {
          std::cerr
              << "Managed network status is unavailable.\n";

          return 1;
        }

        std::cout
            << "Managed network: "
            << managed_network_state_name(status->state)
            << '\n';

        return 0;
      }

      if (action != "info")
      {
        command_usage(command);
        return 2;
      }

      const auto capability =
          network_
              ? std::optional<NetworkCapability>(
                    network_->network_capability())
              : client_.network_capability();

      if (!capability)
      {
        std::cerr
            << "Host network information is unavailable\n";

        return 1;
      }

      const auto managed =
          status.value_or(
              ManagedNetworkStatus{});

      std::cout
          << "State:            "
          << network_state_name(capability->state)
          << "\nPrimary IPv4:     "
          << (capability->primary_ipv4.empty()
                  ? "-"
                  : capability->primary_ipv4)
          << "\nInterface:        "
          << (capability->primary_interface.empty()
                  ? "-"
                  : capability->primary_interface)
          << "\nType:             "
          << network_interface_type_name(
                 capability->interface_type)
          << "\nLocal network:    "
          << local_network_state_name(
                 capability->local_network_state)
          << "\nManaged network:  "
          << managed_network_capability_name(
                 managed.capability);

      if (managed.capability ==
          ManagedNetworkCapability::Available)
      {
        std::cout
            << "\nManaged state:    "
            << managed_network_state_name(
                   managed.state);

        if (managed.state ==
            ManagedNetworkState::Running)
        {
          std::cout
              << "\nManaged interface: "
              << managed.interface_name
              << "\nManaged IPv4:      "
              << managed.ipv4
              << "\nManaged SSID:      "
              << managed.ssid;
        }
      }

      std::cout << '\n';

      return 0;
    }

    if (command == "init")
    {
      std::error_code error;

      const auto root =
          std::filesystem::weakly_canonical(
              std::filesystem::current_path(),
              error);

      if (error)
      {
        std::cerr
            << "failed to determine current working directory\n";

        return 1;
      }

      const auto path =
          root / "softadastra.toml";

      if (std::filesystem::exists(
              path,
              error))
      {
        std::cout
            << "Softadastra project already initialized:\n\n"
            << "  "
            << path.string()
            << '\n';

        return 0;
      }

      std::string name =
          root.filename().string();

      std::string configured_command;
      std::optional<AccessPoint> access;

      for (int index = 2;
           index < argc;
           ++index)
      {
        const std::string value(argv[index]);

        if (value == "--command" &&
            index + 1 < argc)
        {
          configured_command =
              argv[++index];
        }
        else if (
            value == "--access" &&
            index + 1 < argc)
        {
          access =
              parse_access(
                  argv[++index]);

          if (!access)
          {
            std::cerr
                << "access point must use http:port or https:port with port "
                   "1 to 65535\n";

            return 2;
          }
        }
        else if (
            !value.starts_with("-") &&
            name == root.filename().string())
        {
          name = value;
        }
        else
        {
          std::cerr
              << "invalid init arguments\n";

          return 2;
        }
      }

      ProjectIdentity id =
          ProjectIdentity::generate();

      const auto legacy =
          ProjectIdentity::find(root);

      if (legacy &&
          client_.host_available())
      {
        const auto entry =
            client_.software_by_project_identity(
                legacy->second);

        if (entry)
        {
          id =
              ProjectIdentity(
                  entry->id().value());
        }
      }

      if (!ProjectConfigFile::create(
              root,
              ProjectConfig{
                  id,
                  name,
                  configured_command,
                  access,
                  {}}))
      {
        std::cerr
            << "failed to create "
            << path.string()
            << '\n';

        return 1;
      }

      std::cout
          << "Created softadastra.toml\n";

      if (configured_command.empty())
      {
        std::cout
            << "\nSet `command` in softadastra.toml, then run:\n\n"
            << "  softadastra run\n";
      }

      return 0;
    }

    if (!client_.host_available())
    {
      std::cerr
          << "Softadastra Host is unavailable\n";

      return 1;
    }

    if (command == "list")
    {
      if (argc > 3 ||
          (argc == 3 &&
           std::string(argv[2]) != "--running" &&
           std::string(argv[2]) != "--stopped"))
      {
        std::cerr
            << "list accepts only --running or --stopped\n";

        return 2;
      }

      const auto filter =
          argc == 3
              ? std::optional<SoftwareState>(
                    std::string(argv[2]) == "--running"
                        ? SoftwareState::Running
                        : SoftwareState::Stopped)
              : std::nullopt;

      std::cout
          << std::left
          << std::setw(12)
          << "NAME"
          << std::setw(11)
          << "STATE"
          << std::setw(13)
          << "ACCESS"
          << "PROJECT\n";

      for (const auto &entry : client_.software())
      {
        if (!filter ||
            entry.state() == *filter)
        {
          std::cout
              << std::left
              << std::setw(12)
              << entry.id().value()
              << std::setw(11)
              << state_name(entry.state())
              << std::setw(13)
              << access_name(entry.access_point())
              << entry.process_spec()
                     .working_directory()
                     .value_or("-")
              << '\n';
        }
      }

      return 0;
    }

    if (command == "connectivity")
    {
      if (argc != 2)
      {
        command_usage(command);
        return 2;
      }

      std::cout
          << "network: "
          << (client_.connectivity_available()
                  ? "available"
                  : "unavailable")
          << '\n';

      if (client_.connectivity_available())
      {
        std::cout
            << "connected: "
            << (client_.connected()
                    ? "yes"
                    : "no")
            << '\n';
      }

      return 0;
    }

    if (command == "remote")
    {
      if ((argc != 3 ||
           (std::string(argv[2]) != "disable" &&
            std::string(argv[2]) != "status")) &&
          (argc != 5 ||
           std::string(argv[2]) != "enable"))
      {
        command_usage(command);
        return 2;
      }

      const auto response =
          client_.request(
              argc == 3
                  ? (std::string(argv[2]) == "status"
                         ? "remote status"
                         : "remote disable")
                  : "remote enable " +
                        std::string(argv[3]) +
                        " " +
                        argv[4]);

      if (!response ||
          *response == "error")
      {
        std::cerr
            << "failed to update remote reachability\n";

        return 1;
      }

      if (std::string(argv[2]) == "status")
      {
        std::cout
            << *response
            << '\n';

        return 0;
      }

      std::cout
          << (*response == "remote 1"
                  ? "remote reachability: enabled\n"
                  : "remote reachability: disabled\n");

      return 0;
    }

    if (command == "register")
    {
      if (argc < 4)
      {
        command_usage(command);
        return 2;
      }

      int executable = 3;
      std::optional<AccessPoint> access;

      if (std::string(argv[3]) == "--access")
      {
        if (argc < 7 ||
            std::string(argv[5]) != "--")
        {
          command_usage(command);
          return 2;
        }

        access =
            parse_access(argv[4]);

        if (!access)
        {
          std::cerr
              << "access point must use http:port or https:port with port "
                 "1 to 65535\n";

          return 2;
        }

        executable = 6;
      }
      else if (std::string(argv[3]) == "--")
      {
        if (argc < 5)
        {
          command_usage(command);
          return 2;
        }

        executable = 4;
      }

      const std::string software_name(
          argv[2]);

      const SoftwareId id(
          software_name);

      if (client_.software(id))
      {
        std::cerr
            << "Software already registered: "
            << id.value()
            << "\n\nChoose a different software name.\n";

        return 1;
      }

      std::vector<std::string> arguments;

      for (int index = executable + 1;
           index < argc;
           ++index)
      {
        arguments.emplace_back(
            argv[index]);
      }

      std::error_code error;

      const auto root =
          std::filesystem::weakly_canonical(
              std::filesystem::current_path(),
              error);

      if (error)
      {
        return 1;
      }

      if (!client_.register_software(
              id,
              ProcessSpec(
                  argv[executable],
                  std::move(arguments),
                  root.string()),
              access,
              std::nullopt,
              software_name))
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

    const bool valid =
        command == "run" ||
        command == "start" ||
        command == "stop" ||
        command == "restart" ||
        command == "status" ||
        command == "info" ||
        command == "access" ||
        command == "logs" ||
        command == "remove";

    if (!valid)
    {
      std::cerr
          << "Unknown command: "
          << command
          << '\n';

      usage();

      return 2;
    }

    bool follow = false;
    bool clear = false;
    std::optional<std::string> name;

    if (command == "logs")
    {
      for (int index = 2;
           index < argc;
           ++index)
      {
        const std::string value(
            argv[index]);

        if (value == "-f" ||
            value == "--follow")
        {
          follow = true;
        }
        else if (value == "--clear")
        {
          clear = true;
        }
        else if (!name)
        {
          name = value;
        }
        else
        {
          command_usage(command);
          return 2;
        }
      }

      if (follow &&
          clear)
      {
        std::cerr
            << "--clear cannot be used with --follow\n";

        return 2;
      }
    }
    else
    {
      if (argc > 3)
      {
        command_usage(command);
        return 2;
      }

      name =
          argc == 3
              ? std::optional<std::string>(argv[2])
              : std::nullopt;
    }

    if (command == "run" &&
        !name)
    {
      std::string error;

      const auto config =
          ProjectConfigFile::find(
              std::filesystem::current_path(),
              &error);

      if (!error.empty())
      {
        std::cerr
            << error
            << '\n';

        return 1;
      }

      if (config &&
          config->second.command.empty())
      {
        std::cerr
            << "No command configured for: "
            << config->second.name
            << "\n\nSet `command` in:\n\n"
            << "  "
            << (config->first / "softadastra.toml").string()
            << '\n';

        return 1;
      }

      if (config &&
          !client_.software(
              SoftwareId(
                  config->second.id.value())))
      {
        const auto legacy =
            ProjectIdentity::find(
                config->first);

        const auto identity =
            legacy
                ? legacy->second
                : config->second.id;

        if (!client_.register_software(
                SoftwareId(
                    config->second.id.value()),
                shell_process(
                    config->second.command,
                    config->first.string()),
                config->second.access,
                identity,
                config->second.name))
        {
          std::cerr
              << "Failed to start software: "
              << config->second.name
              << '\n';

          return 1;
        }
      }
    }

    auto target =
        resolve_target(
            client_,
            name,
            command);

    if (!target)
    {
      return 1;
    }

    if (command == "run" &&
        !sync_project(
            client_,
            *target))
    {
      return 1;
    }

    if (command == "remove")
    {
      if (target->entry->state() ==
          SoftwareState::Running)
      {
        std::cerr
            << "Cannot remove running software: "
            << target->name
            << "\n\nStop it first:\n\n"
            << "  softadastra stop"
            << (name
                    ? " " + *name
                    : "")
            << "\n";

        return 1;
      }

      if (!client_.remove_software(
              target->id))
      {
        unknown_software(
            target->name);

        return 1;
      }

      std::cout
          << "removed: "
          << target->name
          << '\n';

      return 0;
    }

    if (command == "logs")
    {
      const auto path =
          NativeDataDirectory::path() /
          "logs" /
          (target->id.value() + ".log");

      if (clear)
      {
        std::ofstream output(
            path,
            std::ios::trunc);

        return 0;
      }

      std::uintmax_t offset = 0;

      do
      {
        std::ifstream input(
            path,
            std::ios::binary);

        if (input)
        {
          input.seekg(
              static_cast<std::streamoff>(offset));

          std::cout
              << input.rdbuf()
              << std::flush;

          const auto size =
              std::filesystem::file_size(path);

          offset = size;
        }

        if (!follow)
        {
          break;
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(150));

      } while (true);

      return 0;
    }

    if (command == "access")
    {
      return print_access(
                 client_,
                 *target)
                 ? 0
                 : 1;
    }

    if (command == "info")
    {
      const auto &entry =
          *target->entry;

      std::cout
          << "Name:       "
          << target->name
          << "\nState:      "
          << state_name(entry.state())
          << "\nCommand:    "
          << (target->config
                  ? target->config->command
                  : command_name(entry))
          << "\nProject:    "
          << (target->root
                  ? target->root->string()
                  : entry.process_spec()
                        .working_directory()
                        .value_or("-"))
          << "\nAccess:     "
          << access_name(
                 target->config
                     ? target->config->access
                     : entry.access_point())
          << "\nPID:        "
          << (entry.pid()
                  ? std::to_string(*entry.pid())
                  : "-")
          << "\n";

      return 0;
    }

    if (command == "status")
    {
      const auto state =
          client_.software_state(
              target->id);

      if (!state)
      {
        unknown_software(
            target->name);

        return 1;
      }

      std::cout
          << target->name
          << ": "
          << state_name(*state)
          << '\n';

      return 0;
    }

    if (command == "start" ||
        command == "run")
    {
      const auto state =
          client_.software_state(
              target->id);

      const bool running =
          state &&
          *state == SoftwareState::Running;

      if (!running)
      {
        const auto result =
            client_.start_software(
                target->id);

        if (!result)
        {
          std::cerr
              << "Failed to start software: "
              << target->name
              << "\n\nCommand:\n"
              << "  "
              << (target->config
                      ? target->config->command
                      : command_name(*target->entry))
              << "\n\nProject:\n"
              << "  "
              << (target->root
                      ? target->root->string()
                      : target->entry
                            ->process_spec()
                            .working_directory()
                            .value_or("-"))
              << "\n\nReason:\n"
              << "  ";

          operation_error(result);

          std::cerr << '\n';

          return 1;
        }
      }

      std::cout
          << (running
                  ? "already running: "
                  : (command == "run"
                         ? "running: "
                         : "started: "))
          << target->name
          << '\n';

      if (command == "run" &&
          (target->config
               ? target->config->access.has_value()
               : target->entry
                     ->access_point()
                     .has_value()))
      {
        static_cast<void>(
            print_access(
                client_,
                *target));
      }

      return 0;
    }

    const auto result =
        command == "stop"
            ? client_.stop_software(
                  target->id)
            : client_.restart_software(
                  target->id);

    if (!result)
    {
      std::cerr
          << "Failed to "
          << command
          << " software: "
          << target->name
          << "\n\nReason:\n"
          << "  ";

      operation_error(result);

      std::cerr << '\n';

      return 1;
    }

    std::cout
        << (command == "stop"
                ? "stopped: "
                : "restarted: ")
        << target->name
        << '\n';

    return 0;
  }

} // namespace softadastra
