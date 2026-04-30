/*
 * StoreCommand.cpp
 */

#include "commands/store/StoreCommand.hpp"

#include <iostream>
#include <string>

#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::store
{
  namespace ui = softadastra::cli::utils::ui;

  StoreCommand::StoreCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime),
        put_(runtime),
        get_(runtime)
  {
  }

  cli_types::CliErrorCode StoreCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.empty())
    {
      std::cout << "Softadastra store\n\n";

      std::cout << "Usage\n";
      std::cout << "  softadastra store put <key> <value>\n";
      std::cout << "  softadastra store get <key>\n\n";

      std::cout << "Commands\n";
      std::cout << "  put      Write a key/value pair\n";
      std::cout << "  get      Read one key\n";

      return cli_types::CliErrorCode::None;
    }

    const std::string &subcommand = command.args[0];

    cli_parser::ParsedCommand forwarded = command;

    if (!forwarded.args.empty())
    {
      forwarded.args.erase(forwarded.args.begin());
    }

    if (subcommand == "put")
    {
      return put_.handle(forwarded);
    }

    if (subcommand == "get")
    {
      return get_.handle(forwarded);
    }

    ui::err_line(
        std::cerr,
        "Unknown store command: " + subcommand);

    ui::tip_line(
        std::cerr,
        "Usage: store <put|get>");

    return cli_types::CliErrorCode::CommandNotFound;
  }

} // namespace softadastra::app::cli::commands::store
