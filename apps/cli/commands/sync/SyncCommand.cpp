/*
 * SyncCommand.cpp
 */

#include "commands/sync/SyncCommand.hpp"

#include <iostream>
#include <string>

#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::sync
{
  namespace ui = softadastra::cli::utils::ui;

  SyncCommand::SyncCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime),
        status_(runtime),
        tick_(runtime)
  {
  }

  cli_types::CliErrorCode SyncCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.empty())
    {
      std::cout << "Softadastra sync\n\n";

      std::cout << "Usage\n";
      std::cout << "  softadastra sync status\n";
      std::cout << "  softadastra sync tick\n\n";

      std::cout << "Commands\n";
      std::cout << "  status   Show sync engine state\n";
      std::cout << "  tick     Run one manual sync cycle\n";

      return cli_types::CliErrorCode::None;
    }

    const std::string &subcommand = command.args[0];

    cli_parser::ParsedCommand forwarded = command;

    if (!forwarded.args.empty())
    {
      forwarded.args.erase(forwarded.args.begin());
    }

    if (subcommand == "status")
    {
      return status_.handle(forwarded);
    }

    if (subcommand == "tick")
    {
      return tick_.handle(forwarded);
    }

    ui::err_line(
        std::cerr,
        "Unknown sync command: " + subcommand);

    ui::tip_line(
        std::cerr,
        "Usage: sync <status|tick>");

    return cli_types::CliErrorCode::CommandNotFound;
  }

} // namespace softadastra::app::cli::commands::sync
