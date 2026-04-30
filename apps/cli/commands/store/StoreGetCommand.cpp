/*
 * StoreGetCommand.cpp
 */

#include "commands/store/StoreGetCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>
#include <softadastra/store/types/Key.hpp>

namespace softadastra::app::cli::commands::store
{
  namespace store_types = softadastra::store::types;
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;

  StoreGetCommand::StoreGetCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StoreGetCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.empty())
    {
      ui::err_line(std::cerr, "Missing key argument.");
      ui::tip_line(std::cerr, "Usage: store-get <key>");

      return cli_types::CliErrorCode::MissingArgument;
    }

    const std::string &key_arg = command.args[0];

    if (key_arg.empty())
    {
      ui::err_line(std::cerr, "Key cannot be empty.");

      return cli_types::CliErrorCode::InvalidArguments;
    }

    const store_types::Key key{key_arg};

    const auto entry =
        runtime_.store().get(key);

    if (!entry.has_value())
    {
      ui::err_line(
          std::cerr,
          "Key not found: " + key_arg);

      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    const auto &stored_entry = entry.value();

    ui::section(std::cout, "Store entry");

    const std::vector<std::string> headers{
        "Field",
        "Value",
    };

    const std::vector<std::vector<std::string>> rows{
        std::vector<std::string>{
            "key",
            stored_entry.key.value(),
        },
        std::vector<std::string>{
            "value",
            stored_entry.value.to_string(),
        },
        std::vector<std::string>{
            "version",
            std::to_string(stored_entry.version),
        },
        std::vector<std::string>{
            "timestamp",
            std::to_string(stored_entry.timestamp.millis()),
        },
    };

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::store
