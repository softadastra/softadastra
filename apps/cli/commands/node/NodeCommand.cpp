/*
 * NodeCommand.cpp
 */

#include "commands/node/NodeCommand.hpp"

#include <iostream>
#include <string>

#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::node
{
  namespace ui = softadastra::cli::utils::ui;

  NodeCommand::NodeCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime),
        info_(runtime),
        start_(runtime)
  {
  }

  cli_types::CliErrorCode NodeCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.empty())
    {
      std::cout << "Softadastra node\n\n";

      std::cout << "Usage\n";
      std::cout << "  softadastra node info\n";
      std::cout << "  softadastra node start\n\n";

      std::cout << "Commands\n";
      std::cout << "  info     Show local node information\n";
      std::cout << "  start    Start local node services for this CLI session\n";

      return cli_types::CliErrorCode::None;
    }

    const std::string &subcommand = command.args[0];

    cli_parser::ParsedCommand forwarded = command;

    if (!forwarded.args.empty())
    {
      forwarded.args.erase(forwarded.args.begin());
    }

    if (subcommand == "info")
    {
      return info_.handle(forwarded);
    }

    if (subcommand == "start")
    {
      return start_.handle(forwarded);
    }

    ui::err_line(
        std::cerr,
        "Unknown node command: " + subcommand);

    ui::tip_line(
        std::cerr,
        "Usage: node <info|start>");

    return cli_types::CliErrorCode::CommandNotFound;
  }

} // namespace softadastra::app::cli::commands::node
