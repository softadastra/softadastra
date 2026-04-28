/*
 * NodeInfoCommand.cpp
 */

#include "commands/node/NodeInfoCommand.hpp"

#include <iostream>

namespace softadastra::app::cli::commands::node
{
  NodeInfoCommand::NodeInfoCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode NodeInfoCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    auto metadata = runtime_.metadata().local();

    if (!metadata.has_value())
    {
      metadata = runtime_.metadata().local_or_refresh();
    }

    if (!metadata.has_value())
    {
      std::cerr << "Unable to read local node metadata.\n";
      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    std::cout << "Softadastra node\n\n";

    std::cout << "node_id: "
              << metadata->identity.node_id
              << "\n";

    std::cout << "display_name: "
              << metadata->identity.display_name
              << "\n";

    std::cout << "hostname: "
              << metadata->runtime.hostname
              << "\n";

    std::cout << "os: "
              << metadata->runtime.os_name
              << "\n";

    std::cout << "version: "
              << metadata->runtime.version
              << "\n";

    std::cout << "started_at: "
              << metadata->runtime.started_at
              << "\n";

    std::cout << "uptime_ms: "
              << metadata->runtime.uptime_ms
              << "\n";

    std::cout << "capabilities: "
              << metadata->capabilities.values.size()
              << "\n";

    std::cout << "node_running: "
              << (runtime_.node_running() ? "yes" : "no")
              << "\n";

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::node
