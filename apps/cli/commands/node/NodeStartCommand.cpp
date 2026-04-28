/*
 * NodeStartCommand.cpp
 */

#include "commands/node/NodeStartCommand.hpp"

#include <iostream>

namespace softadastra::app::cli::commands::node
{
  NodeStartCommand::NodeStartCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode NodeStartCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    if (runtime_.node_running())
    {
      std::cout << "Softadastra node already running.\n";
      return cli_types::CliErrorCode::None;
    }

    if (!runtime_.start_node())
    {
      std::cerr << "Failed to start Softadastra node.\n";
      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    std::cout << "Softadastra node started.\n";
    std::cout << "node_id: " << runtime_.node_id() << "\n";
    std::cout << "transport: running\n";
    std::cout << "discovery: running\n";
    std::cout << "metadata: running\n";

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::node
