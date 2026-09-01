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

#include "host/HostObservation.hpp"
#include "platform/NativeDataDirectory.hpp"

#include "cli/AccessUrl.hpp"
#include "cli/CliStyle.hpp"
#include "platform/ProcessSpec.hpp"
#include "platform/QrCode.hpp"
#include "software/AccessPoint.hpp"
#include "software/ProjectConfig.hpp"
#include "software/ProjectIdentity.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace
{
  using namespace softadastra;

  namespace style = softadastra::cli::style;

  // These are the historical widths of the individual CLI output blocks.
  // They intentionally differ: output compatibility takes precedence over
  // visually uniform columns.
  constexpr std::size_t access_field_width = 15;
  constexpr std::size_t network_start_field_width = 11;
  constexpr std::size_t managed_network_field_width = 17;
  constexpr std::size_t network_info_field_width = 18;
  constexpr std::size_t managed_network_info_field_width = 19;
  constexpr std::size_t software_info_field_width = 12;

  // Styles a state word without changing the word itself: color only reinforces
  // the always-present text, never replaces it.
  std::string state_display(
      SoftwareState state,
      style::Stream stream = style::Stream::Out)
  {
    const char *const name =
        software_state_name(state);

    switch (state)
    {
    case SoftwareState::Running:
      return style::success(name, stream);

    case SoftwareState::Failed:
      return style::error(name, stream);

    case SoftwareState::Stopped:
    case SoftwareState::Starting:
      return style::muted(name, stream);
    }

    return name;
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

    case SoftwareOperationError::LocalAccessUnavailable:
      std::cerr << "local access could not be opened on this Host";
      break;
    }
  }

  std::string operation_result_text(
      const SoftwareOperationResult &result)
  {
    switch (result.error().value_or(SoftwareOperationError::LaunchFailed))
    {
    case SoftwareOperationError::ProcessExitedSuccessfully: return "exited successfully";
    case SoftwareOperationError::ProcessExitedWithNonZeroCode: return "process exited with code " + std::to_string(result.exit_code().value_or(-1));
    case SoftwareOperationError::SoftwareUnknown: return "software is unknown";
    case SoftwareOperationError::AlreadyRunning: return "software is already running";
    case SoftwareOperationError::NotRunning: return "software is not running";
    case SoftwareOperationError::ExecutableNotFound: return "command could not be started";
    case SoftwareOperationError::PermissionDenied: return "permission denied";
    case SoftwareOperationError::LaunchFailed: return "launch failed";
    case SoftwareOperationError::StopFailed: return "stop failed";
    case SoftwareOperationError::LocalAccessUnavailable: return "local access could not be opened on this Host";
    }
    return "launch failed";
  }

  void print_last_result(
      const std::optional<SoftwareOperationResult> &result,
      const SoftwareState state,
      const std::size_t width)
  {
    if (result)
    {
      std::cout << '\n' << style::field(
          state == SoftwareState::Failed ? "Reason:" : "Result:",
          operation_result_text(*result), width);
    }
  }

  void launch_output_hint(
      const SoftwareId &id)
  {
    const auto path =
        NativeDataDirectory::path() /
        "logs" /
        (id.value() + ".log");
    std::ifstream input(path);

    if (!input)
    {
      return;
    }

    std::string line;
    std::string last;

    while (std::getline(input, line))
    {
      if (!line.empty())
      {
        last = line;
      }
    }

    if (!last.empty())
    {
      std::cerr
          << "\n\nProcess output:\n"
          << "  "
          << last
          << "\n\nFull output:\n"
          << "  "
          << style::command(
                 "softadastra logs " + id.value(),
                 style::Stream::Err);
    }
  }

  void unknown_software(
      const std::string &name)
  {
    std::cerr
        << "Software not found: "
        << name
        << "\n\nView registered software with:\n\n"
        << "  "
        << style::command(
               "softadastra list",
               style::Stream::Err)
        << "\n";
  }

  std::optional<SoftwareEntry> software_by_name(
      const ControlClient &client,
      const std::string &name)
  {
    const auto entries =
        client.software();

    for (const auto &entry : entries)
    {
      if (entry.name() == name)
      {
        return entry;
      }
    }

    // Legacy registrations used their identifier as their visible name.
    for (const auto &entry : entries)
    {
      if (entry.id().value() == name)
      {
        return entry;
      }
    }

    return std::nullopt;
  }

  std::filesystem::path normalized_project_root(
      const std::filesystem::path &path)
  {
    std::error_code error;
    const auto canonical =
        std::filesystem::weakly_canonical(
            path,
            error);

    if (!error)
    {
      return canonical;
    }

    error.clear();
    const auto absolute =
        std::filesystem::absolute(
            path,
            error);

    return (error ? path : absolute).lexically_normal();
  }

  std::optional<SoftwareEntry> software_by_project_root(
      const ControlClient &client,
      const std::filesystem::path &root)
  {
    const auto normalized_root =
        normalized_project_root(root);

    for (const auto &entry : client.software())
    {
      const auto working_directory =
          entry.process_spec().working_directory();

      if (working_directory &&
          normalized_project_root(*working_directory) == normalized_root)
      {
        return entry;
      }
    }

    return std::nullopt;
  }

  void software_name_conflict(
      const std::string &name,
      const SoftwareEntry &existing,
      const std::filesystem::path &current_root)
  {
    std::cerr
        << "Software name already registered: "
        << name
        << "\n\nExisting project:\n  "
        << existing.process_spec().working_directory().value_or("unavailable")
        << "\n\nCurrent project:\n  "
        << current_root.string()
        << "\n\nChoose another name or remove the existing registration.\n";
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
          << "  "
          << style::command(
                 "softadastra " + command + " <name>",
                 style::Stream::Err)
          << "\n";

      return;
    }

    std::cerr
        << "No Softadastra project found.\n\n"
        << "Initialize the current project with:\n\n"
        << "  "
        << style::command(
               "softadastra init",
               style::Stream::Err)
        << "\n\nOr select a registered software explicitly:\n\n"
        << "  "
        << style::command(
               "softadastra " + command + " <name>",
               style::Stream::Err)
        << "\n";
  }

  void usage()
  {
    std::ostream &out = std::cout;

    out << "Usage:\n";
    out << "  softadastra <command> [arguments]\n";
    out << "  softadastra help <command>\n\n";

    out << "Run software on this Host.\n\n";

    out << "Project:\n";
    out << "  init          [name]               Create softadastra.toml\n";
    out << "  run           [name]               Run software\n\n";

    out << "Software:\n";
    out << "  start         [name]               Start registered software\n";
    out << "  stop          [name]               Stop software\n";
    out << "  restart       [name]               Restart software\n";
    out << "  status        [name]               Show software status\n";
    out << "  info          [name]               Show software information\n";
    out << "  access        [name]               Show local access\n";
    out << "  access        allow                Allow local access\n";
    out << "  access        deny                 Deny local access\n";
    out << "  logs          [name]               Show software logs\n";
    out << "  remove        [name]               Remove software from this Host\n\n";

    out << "Inventory:\n";
    out << "  list                               List registered software\n\n";

    out << "Host:\n";
    out << "  connectivity                      Show Host connectivity\n";
    out << "  network       <command>            Manage local networking\n";
    out << "  remote        <command>            Manage remote access\n\n";

    out << "Advanced:\n";
    out << "  register      <name>               Register software directly\n\n";

    out << "Options:\n";
    out << "  -h, --help                         Show this help\n\n";

    out << "Run 'softadastra help <command>' for detailed command usage.\n";
  }

  void command_usage(
      const std::string &command)
  {
    std::ostream &out = std::cout;

    if (command == "logs")
    {
      out << "Usage:\n";
      out << "  softadastra logs [name] [options]\n\n";

      out << "Show stored output for software.\n\n";

      out << "Without a name, logs for the current project are shown.\n";
      out << "With a name, logs for registered software are shown.\n\n";

      out << "Options:\n";
      out << "  -f, --follow    Follow new log output\n";
      out << "      --clear     Clear stored logs\n";
    }
    else if (command == "access")
    {
      out << "Usage:\n";
      out << "  softadastra access [name]\n";
      out << "  softadastra access <command>\n\n";

      out << "Show or manage local access for software.\n\n";

      out << "Commands:\n";
      out << "  allow           Allow local access for the current project\n";
      out << "  deny            Deny local access for the current project\n\n";

      out << "Without a command, show the current local access state.\n";
    }
    else if (command == "network")
    {
      out << "Usage:\n";
      out << "  softadastra network <command>\n\n";

      out << "Show and manage local networking on this Host.\n\n";

      out << "Commands:\n";
      out << "  info            Show Host network information\n";
      out << "  status          Show managed network status\n";
      out << "  start           Start the managed local network\n";
      out << "  stop            Stop the managed local network\n";
    }
    else if (command == "init")
    {
      out << "Usage:\n";
      out << "  softadastra init [name] [options]\n\n";

      out << "Create softadastra.toml for the current project.\n\n";

      out << "Options:\n";
      out << "  --command <command>    Set the software command\n";
      out << "  --access <http:port>   Set local HTTP access\n";
    }
    else if (command == "run")
    {
      out << "Usage:\n";
      out << "  softadastra run [name]\n\n";

      out << "Run software on this Host.\n\n";

      out << "Without a name, run the current Softadastra project.\n";
      out << "With a name, run registered software.\n";
    }
    else if (command == "list")
    {
      out << "Usage:\n";
      out << "  softadastra list [options]\n\n";

      out << "List software registered on this Host.\n\n";

      out << "Options:\n";
      out << "  --running    Show only running software\n";
      out << "  --stopped    Show only stopped software\n";
    }
    else if (command == "register")
    {
      out << "Usage:\n";
      out << "  softadastra register <name> [options] -- <command> [arguments...]\n\n";

      out << "Register software directly on this Host.\n\n";

      out << "Options:\n";
      out << "  --access <http:port>   Set local HTTP access\n";
    }
    else if (command == "connectivity")
    {
      out << "Usage:\n";
      out << "  softadastra connectivity\n\n";

      out << "Show Host connectivity.\n";
    }
    else if (command == "remote")
    {
      out << "Usage:\n";
      out << "  softadastra remote <command>\n\n";

      out << "Manage remote access to this Host.\n\n";

      out << "Commands:\n";
      out << "  status                         Show remote access status\n";
      out << "  enable <ipv4-address> <port>   Enable remote access\n";
      out << "  disable                        Disable remote access\n";
    }
    else if (command == "start")
    {
      out << "Usage:\n";
      out << "  softadastra start [name]\n\n";

      out << "Start registered software.\n";
    }
    else if (command == "stop")
    {
      out << "Usage:\n";
      out << "  softadastra stop [name]\n\n";

      out << "Stop running software.\n";
    }
    else if (command == "restart")
    {
      out << "Usage:\n";
      out << "  softadastra restart [name]\n\n";

      out << "Restart software.\n";
    }
    else if (command == "status")
    {
      out << "Usage:\n";
      out << "  softadastra status [name]\n\n";

      out << "Show software status.\n";
    }
    else if (command == "info")
    {
      out << "Usage:\n";
      out << "  softadastra info [name]\n\n";

      out << "Show software information.\n";
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

  std::optional<AccessPoint> effective_access(
      const ProjectConfig &config)
  {
    if (config.access)
    {
      return config.access;
    }

    return config.access_points.empty()
               ? std::nullopt
               : std::optional<AccessPoint>(
                     config.access_points.front());
  }

  std::optional<AccessPoint> effective_access(
      const Target &target)
  {
    return target.config
               ? effective_access(*target.config)
               : target.entry->access_point();
  }

  std::optional<Target> resolve_target(
      ControlClient &client,
      const std::optional<std::string> &name,
      const std::string &command)
  {
    if (name)
    {
      const auto entry =
          software_by_name(
              client,
              *name);

      Target target{
          entry ? entry->id() : SoftwareId(""),
          *name,
          entry,
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
      const auto entry =
          software_by_project_root(
              client,
              config->first);

      Target target{
          entry ? entry->id() : SoftwareId(""),
          config->second.name,
          entry,
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

    const auto same_name =
        software_by_name(
            client,
            target.config->name);

    if (same_name &&
        same_name->id() != target.id)
    {
      software_name_conflict(
          target.config->name,
          *same_name,
          *target.root);

      return false;
    }

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

    target.entry =
        client.software(target.id);

    return true;
  }

  std::string access_owner(const std::filesystem::path &root)
  {
    // A stable tag belongs to the project location, never to TOML content or
    // to a Host SoftwareId. It fits UFW's short rule-comment limit.
    std::uint64_t hash = 1469598103934665603ULL;
    for (const char character : root.string())
    {
      hash ^= static_cast<unsigned char>(character);
      hash *= 1099511628211ULL;
    }
    std::ostringstream tag;
    tag << "softadastra:" << std::hex << hash;
    return tag.str();
  }

  std::optional<LocalFirewallRule> local_firewall_rule(
      const Network *network,
      const std::filesystem::path &root,
      const AccessPoint &access)
  {
    if (network == nullptr)
      return std::nullopt;
    const auto capability = network->network_capability();
    if (capability.local_network_state != LocalNetworkState::Existing ||
        capability.local_subnet.empty())
      return std::nullopt;
    return LocalFirewallRule{
        access_owner(normalized_project_root(root)), capability.local_subnet, access.port()};
  }

  enum class LocalAccessFirewallStatus
  {
    NotChecked,
    Allowed,
    Blocked,
    Unsupported,
    Failed
  };

  LocalAccessFirewallStatus local_firewall_status(
      LocalFirewallResult result) noexcept
  {
    switch (result)
    {
    case LocalFirewallResult::Open:
    case LocalFirewallResult::Disabled:
      return LocalAccessFirewallStatus::Allowed;

    case LocalFirewallResult::PermissionRequired:
      return LocalAccessFirewallStatus::Blocked;

    case LocalFirewallResult::Unsupported:
      return LocalAccessFirewallStatus::Unsupported;

    case LocalFirewallResult::Failed:
      return LocalAccessFirewallStatus::Failed;
    }

    return LocalAccessFirewallStatus::Failed;
  }

  LocalAccessFirewallStatus local_firewall_status(
      LocalAccessFirewallState state) noexcept
  {
    switch (state)
    {
    case LocalAccessFirewallState::Open:
      return LocalAccessFirewallStatus::Allowed;

    case LocalAccessFirewallState::PermissionRequired:
      return LocalAccessFirewallStatus::Blocked;

    case LocalAccessFirewallState::Unsupported:
      return LocalAccessFirewallStatus::Unsupported;

    case LocalAccessFirewallState::Failed:
      return LocalAccessFirewallStatus::Failed;

    case LocalAccessFirewallState::NotRequired:
      return LocalAccessFirewallStatus::NotChecked;
    }

    return LocalAccessFirewallStatus::Failed;
  }

  struct LocalAccessView
  {
    std::optional<AccessPoint> access;
    LocalAccessFirewallStatus firewall{
        LocalAccessFirewallStatus::NotChecked};
    bool firewall_rule_available{true};
  };

  LocalAccessView local_access_view(
      const Target &target,
      const std::optional<LocalAccess> &host_access,
      const Network *network,
      LocalFirewall *firewall)
  {
    LocalAccessView view{
        effective_access(target),
        host_access
            ? local_firewall_status(host_access->firewall)
            : LocalAccessFirewallStatus::NotChecked};

    const bool check_firewall =
        firewall != nullptr &&
        view.access.has_value() &&
        (!host_access ||
         host_access->state == LocalAccessState::Available);

    if (!check_firewall)
    {
      return view;
    }

    auto root = target.root;

    if (!root &&
        target.entry &&
        target.entry->process_spec().working_directory())
    {
      root = std::filesystem::path(
          *target.entry->process_spec().working_directory());
    }

    const auto rule =
        root
            ? local_firewall_rule(
                  network,
                  *root,
                  *view.access)
            : std::nullopt;

    view.firewall_rule_available = rule.has_value();

    view.firewall =
        rule
            ? local_firewall_status(
                  firewall->status(*rule))
            : LocalAccessFirewallStatus::Failed;

    return view;
  }

  bool firewall_allows_local_access(
      LocalAccessFirewallStatus status) noexcept
  {
    return status == LocalAccessFirewallStatus::NotChecked ||
           status == LocalAccessFirewallStatus::Allowed;
  }

  void print_local_firewall_reason(
      LocalAccessFirewallStatus status)
  {
    switch (status)
    {
    case LocalAccessFirewallStatus::Blocked:
      std::cout
          << style::warning("Firewall rule is blocked.")
          << "\n\nRun:\n\n"
          << "  "
          << style::command("softadastra access allow")
          << "\n";
      return;

    case LocalAccessFirewallStatus::Unsupported:
      std::cout
          << style::muted(
                 "Local firewall access is unsupported on this Host.")
          << '\n';
      return;

    case LocalAccessFirewallStatus::Failed:
      std::cout
          << style::muted(
                 "Local firewall access could not be confirmed.")
          << '\n';
      return;

    case LocalAccessFirewallStatus::NotChecked:
    case LocalAccessFirewallStatus::Allowed:
      return;
    }
  }

  int print_firewall_access(
      const Network *network,
      LocalFirewall *firewall,
      const Target &target)
  {
    if (firewall == nullptr)
    {
      std::cerr << "Local firewall access is unsupported on this Host.\n";
      return 1;
    }
    const auto view = local_access_view(
        target,
        std::nullopt,
        network,
        firewall);
    if (!view.access)
    {
      std::cerr << "No access configured for: " << target.name << '\n';
      return 1;
    }
    if (view.firewall == LocalAccessFirewallStatus::Failed &&
        !view.firewall_rule_available)
    {
      std::cerr << "A usable local network and Access are required.\n";
      return 1;
    }
    std::cout << style::field("Software:", target.name, access_field_width) << '\n'
              << style::field("Access:", access_name(view.access), access_field_width) << '\n';
    if (firewall_allows_local_access(view.firewall))
    {
      const auto ipv4 = network->network_capability().primary_ipv4;
      const auto url = view.access->protocol() == AccessProtocol::Https
                           ? AccessUrl::https(ipv4, view.access->port())
                           : AccessUrl::http(ipv4, view.access->port());
      std::cout << style::field("Local access:", style::success("allowed"), access_field_width) << '\n'
                << style::field("Local URL:", url, access_field_width) << '\n';
      return 0;
    }
    std::cout << style::field("Local access:", style::warning("unavailable"), access_field_width) << '\n';
    print_local_firewall_reason(view.firewall);
    return 1;
  }

  int print_project_access(const Network *network, LocalFirewall *firewall)
  {
    std::string error;
    const auto project = ProjectConfigFile::find(std::filesystem::current_path(), &error);
    if (!error.empty())
    {
      std::cerr << error << '\n';
      return 1;
    }
    if (!project)
    {
      no_project("access");
      return 1;
    }
    return print_firewall_access(
        network,
        firewall,
        Target{SoftwareId(""), project->second.name, std::nullopt,
               project->first, project->second});
  }

  int print_named_access(
      ControlClient &client,
      const Network *network,
      LocalFirewall *firewall,
      const std::string &name)
  {
    const auto entry = software_by_name(client, name);
    if (!entry)
    {
      unknown_software(name);
      return 1;
    }
    const auto root = entry->process_spec().working_directory();
    if (!root)
    {
      std::cerr << "No project Access configured for: " << name << '\n';
      return 1;
    }
    return print_firewall_access(
        network,
        firewall,
        Target{entry->id(), entry->name(), entry, *root, std::nullopt});
  }

  int change_project_access(
      const std::string &action,
      const Network *network,
      LocalFirewall *firewall)
  {
    if (firewall == nullptr)
    {
      std::cerr << "Local firewall access is unsupported on this Host.\n";
      return 1;
    }
    std::string error;
    const auto project = ProjectConfigFile::find(std::filesystem::current_path(), &error);
    if (!error.empty())
    {
      std::cerr << error << '\n';
      return 1;
    }
    if (!project)
    {
      no_project("access " + action);
      return 1;
    }
    const auto access = effective_access(project->second);
    const auto rule = access
                          ? local_firewall_rule(
                                network,
                                project->first,
                                *access)
                          : std::nullopt;
    if (!rule)
    {
      std::cerr << "A usable local network and Access are required.\n";
      return 1;
    }
    const auto result = action == "allow" ? firewall->allow(*rule) : firewall->deny(*rule);
    const auto firewall_status = local_firewall_status(result);
    if (firewall_status == LocalAccessFirewallStatus::Blocked)
    {
      std::cerr << "Firewall authorization was cancelled or denied.\n";
      return 1;
    }
    if (firewall_status == LocalAccessFirewallStatus::Unsupported)
    {
      std::cerr << "Local firewall access is unsupported on this Host.\n";
      return 1;
    }
    if (firewall_status != LocalAccessFirewallStatus::Allowed)
    {
      std::cerr << "Local firewall access could not be changed.\n";
      return 1;
    }
    if (action == "deny")
    {
      std::cout << style::success("Local access denied.") << '\n';
      return 0;
    }
    const auto ipv4 = network->network_capability().primary_ipv4;
    const auto url = access->protocol() == AccessProtocol::Https
                         ? AccessUrl::https(ipv4, access->port())
                         : AccessUrl::http(ipv4, access->port());
    std::cout << style::success("Local access allowed.") << '\n'
              << url << '\n';
    return 0;
  }

  bool print_access(
      ControlClient &client,
      const Target &target,
      const Network *network = nullptr,
      LocalFirewall *firewall = nullptr)
  {
    const auto configured =
        effective_access(target);

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

    const auto view = local_access_view(
        target,
        access,
        network,
        firewall);

    std::cout
        << style::field("Software:", target.name, access_field_width)
        << '\n'
        << style::field(
               "State:",
               state_display(entry->state()),
               access_field_width)
        << '\n'
        << style::field(
               "Access:",
               access_name(configured),
               access_field_width)
        << '\n';

    if (access->state == LocalAccessState::Available)
    {
      if (!firewall_allows_local_access(view.firewall))
      {
        std::cout
            << style::field(
                   "Local access:",
                   style::warning("unavailable"),
                   access_field_width)
            << '\n';
        print_local_firewall_reason(view.firewall);
        return false;
      }
      std::cout
          << style::field(
                 "Network:",
                 local_access_network_name(access->network),
                 access_field_width)
          << '\n'
          << style::field(
                 "Local URL:",
                 access->url,
                 access_field_width)
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
        << style::field(
               "Local access:",
               style::warning("unavailable"),
               access_field_width)
        << '\n';

    if (!firewall_allows_local_access(view.firewall))
    {
      std::cout << '\n';
      print_local_firewall_reason(view.firewall);
    }

    if (entry->state() == SoftwareState::Stopped)
    {
      std::cout
          << "\nStart it with:\n\n"
          << "  "
          << style::command(
                 target.root
                     ? "softadastra run"
                     : "softadastra run " + target.name)
          << "\n";
    }
    else if (entry->state() == SoftwareState::Failed)
    {
      std::cout
          << "\nInspect logs with:\n\n"
          << "  "
          << style::command(
                 target.root
                     ? "softadastra logs"
                     : "softadastra logs " + target.name)
          << "\n";
    }
    else if (access->managed_network_start_failed)
    {
      std::cout
          << "\n"
          << style::warning("Unable to start a local Softadastra network.")
          << '\n';
    }
    else if (
        access->local_network_state ==
        LocalNetworkState::Unavailable)
    {
      std::cout
          << "\n"
          << style::muted("No local network is available on this Host.")
          << "\n\n"
          << style::field(
                 "Managed network:",
                 managed_network_capability_name(
                     access->managed_network_capability),
                 managed_network_field_width)
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
      ManagedNetwork &managed_network,
      LocalFirewall &local_firewall) noexcept
      : client_(client),
        network_(&network),
        managed_network_(&managed_network),
        local_firewall_(&local_firewall)
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
        std::ostream &out = std::cout;

        out << "Usage:\n";
        out << "  softadastra remove [name]\n\n";

        out << "Remove software from this Host.\n\n";

        out << "The project files and logs are not deleted.\n";
      }
      else if (
          argc == 3 &&
          std::string(argv[2]) == "logs")
      {
        std::ostream &out = std::cout;

        out << "Usage:\n";
        out << "  softadastra logs [name] [options]\n\n";

        out << "Show stored output for software.\n\n";

        out << "Options:\n";
        out << "  -f, --follow    Follow new log output\n";
        out << "      --clear     Clear stored logs\n";
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
        std::ostream &out = std::cout;

        out << "Usage:\n";
        out << "  softadastra remove [name]\n\n";

        out << "Remove software from this Host.\n\n";

        out << "The project files and logs are not deleted.\n";
      }
      else if (command == "logs")
      {
        std::ostream &out = std::cout;

        out << "Usage:\n";
        out << "  softadastra logs [name] [options]\n\n";

        out << "Show stored output for software.\n\n";

        out << "Options:\n";
        out << "  -f, --follow    Follow new log output\n";
        out << "      --clear     Clear stored logs\n";
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
                    ? style::success(
                          "Softadastra local network is already running.")
                    : style::success(
                          "Softadastra local network started."))
            << '\n'
            << style::field("Network:", current->ssid, network_start_field_width)
            << '\n'
            << style::field("IPv4:", current->ipv4, network_start_field_width)
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
              << style::muted(
                     "Softadastra local network is not running.")
              << '\n';

          return 0;
        }

        std::cout
            << style::success(
                   "Softadastra local network stopped.")
            << '\n';

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
            << style::field(
                   "Managed network:",
                   managed_network_state_name(status->state),
                   managed_network_field_width)
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
          << style::field(
                 "State:",
                 network_state_name(capability->state),
                 network_info_field_width)
          << '\n'
          << style::field(
                 "Primary IPv4:",
                 capability->primary_ipv4.empty()
                     ? "-"
                     : capability->primary_ipv4,
                 network_info_field_width)
          << '\n'
          << style::field(
                 "Interface:",
                 capability->primary_interface.empty()
                     ? "-"
                     : capability->primary_interface,
                 network_info_field_width)
          << '\n'
          << style::field(
                 "Type:",
                 network_interface_type_name(
                     capability->interface_type),
                 network_info_field_width)
          << '\n'
          << style::field(
                 "Local network:",
                 local_network_state_name(
                     capability->local_network_state),
                 network_info_field_width)
          << '\n'
          << style::field(
                 "Managed network:",
                 managed_network_capability_name(
                     managed.capability),
                 network_info_field_width);

      if (managed.capability ==
          ManagedNetworkCapability::Available)
      {
        std::cout
            << '\n'
            << style::field(
                   "Managed state:",
                   managed_network_state_name(
                       managed.state),
                   network_info_field_width);

        if (managed.state ==
            ManagedNetworkState::Running)
        {
          std::cout
              << '\n'
              << style::field(
                     "Managed interface:",
                     managed.interface_name,
                     managed_network_info_field_width)
              << '\n'
              << style::field(
                     "Managed IPv4:",
                     managed.ipv4,
                     managed_network_info_field_width)
              << '\n'
              << style::field(
                     "Managed SSID:",
                     managed.ssid,
                     managed_network_info_field_width);
        }
      }

      std::cout << '\n';

      return 0;
    }

    if (local_firewall_ != nullptr && command == "access" && argc == 3 &&
        (std::string(argv[2]) == "allow" || std::string(argv[2]) == "deny"))
    {
      return change_project_access(argv[2], network_, local_firewall_);
    }

    if (local_firewall_ != nullptr && command == "access")
    {
      if (argc == 2)
        return print_project_access(network_, local_firewall_);
      if (argc == 3)
        return print_named_access(client_, network_, local_firewall_, argv[2]);
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

      if (!ProjectConfigFile::create(
              root,
              ProjectConfig{
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
          << style::success("Created softadastra.toml")
          << '\n';

      if (configured_command.empty())
      {
        std::cout
            << "\nSet `command` in softadastra.toml, then run:\n\n"
            << "  "
            << style::command("softadastra run")
            << "\n";
      }

      return 0;
    }

    if (!observe_host(
             client_,
             NativeDataDirectory::path())
             .available())
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

      {
        std::ostringstream header;
        header
            << std::left
            << std::setw(12)
            << "NAME"
            << std::setw(11)
            << "STATE"
            << std::setw(13)
            << "ACCESS"
            << "PROJECT";

        std::cout
            << style::muted(header.str())
            << '\n';
      }

      for (const auto &entry : client_.software())
      {
        if (!filter ||
            entry.state() == *filter)
        {
          // The state column is padded on its plain name so alignment holds,
          // then the visible word is styled in place without shifting columns.
          std::ostringstream row;
          row
              << std::left
              << std::setw(12)
              << entry.name();

          const std::string state_plain =
              software_state_name(entry.state());
          const std::string state_styled =
              state_display(entry.state());

          row << state_styled;
          if (state_plain.size() < 11)
          {
            row << std::string(11 - state_plain.size(), ' ');
          }

          row
              << std::setw(13)
              << access_name(entry.access_point())
              << entry.process_spec()
                     .working_directory()
                     .value_or("-");

          std::cout
              << row.str()
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

      const bool available =
          client_.connectivity_available();

      std::cout
          << style::label("network:")
          << ' '
          << (available
                  ? style::success("available")
                  : style::warning("unavailable"))
          << '\n';

      if (available)
      {
        const bool connected =
            client_.connected();

        std::cout
            << style::label("connected:")
            << ' '
            << (connected
                    ? style::success("yes")
                    : style::muted("no"))
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
          << style::label("remote reachability:")
          << ' '
          << (*response == "remote 1"
                  ? style::success("enabled")
                  : style::muted("disabled"))
          << '\n';

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
          << style::success("registered:")
          << ' '
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
          !software_by_project_root(
              client_,
              config->first))
      {
        const auto same_name =
            software_by_name(
                client_,
                config->second.name);

        if (same_name)
        {
          software_name_conflict(
              config->second.name,
              *same_name,
              config->first);

          return 1;
        }

        if (!client_.register_software(
                SoftwareId::generate(),
                shell_process(
                    config->second.command,
                    config->first.string()),
                effective_access(config->second),
                std::nullopt,
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
      if (!client_.remove_software(
              target->id))
      {
        const auto current = client_.software_state(target->id);
        if (current && *current == SoftwareState::Running)
        {
          std::cerr
              << "Cannot remove running software: "
              << target->name
              << "\n\nStop it first:\n\n"
              << "  "
              << style::command(
                     name
                         ? "softadastra stop " + *name
                         : "softadastra stop",
                     style::Stream::Err)
              << "\n";
          return 1;
        }
        unknown_software(
            target->name);

        return 1;
      }

      std::cout
          << style::success("removed:")
          << ' '
          << target->name
          << '\n';

      return 0;
    }

    if (command == "access")
    {
      return print_access(client_, *target) ? 0 : 1;
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

    if (command == "info")
    {
      const auto &entry =
          *target->entry;

      std::cout
          << style::field("Name:", target->name, software_info_field_width)
          << '\n'
          << style::field(
                 "State:",
                 state_display(entry.state()),
                 software_info_field_width)
          << '\n'
          << style::field(
                 "Command:",
                 target->config
                     ? target->config->command
                     : command_name(entry),
                 software_info_field_width)
          << '\n'
          << style::field(
                 "Project:",
                 target->root
                     ? target->root->string()
                     : entry.process_spec()
                           .working_directory()
                           .value_or("-"),
                 software_info_field_width)
          << '\n'
          << style::field(
                 "Access:",
                 access_name(
                     effective_access(*target)),
                 software_info_field_width)
          << '\n'
          << style::field(
                 "PID:",
                 entry.pid()
                     ? std::to_string(*entry.pid())
                     : "-",
                 software_info_field_width)
          << '\n';
      print_last_result(
          client_.software_result(target->id),
          entry.state(),
          software_info_field_width);
      std::cout << '\n';

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
          << state_display(*state);
      print_last_result(client_.software_result(target->id), *state, 0);
      std::cout << '\n';

      return 0;
    }

    if (command == "start" ||
        command == "run")
    {
      const auto result = client_.start_software(target->id);
      const bool already_running =
          result.error() == SoftwareOperationError::AlreadyRunning;

      if (!result && !already_running)
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

          launch_output_hint(target->id);

          std::cerr << '\n';

        return 1;
      }

      std::cout
          << (already_running
                  ? style::warning("already running:")
                  : (command == "run"
                         ? style::success("running:")
                         : style::success("started:")))
          << ' '
          << target->name
          << '\n';

      if (command == "run" &&
          effective_access(*target).has_value())
      {
        static_cast<void>(
            print_access(
                client_,
                *target,
                network_,
                local_firewall_));
      }

      return 0;
    }

    const auto result =
        command == "stop"
            ? client_.stop_software(
                  target->id)
            : client_.restart_software(
                  target->id);
    const bool already_stopped =
        command == "stop" &&
        result.error() == SoftwareOperationError::NotRunning;

    if (!result && !already_stopped)
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
                ? (already_stopped
                       ? style::warning("already stopped:")
                       : style::success("stopped:"))
                : style::success("restarted:"))
        << ' '
        << target->name
        << '\n';

    return 0;
  }

} // namespace softadastra
