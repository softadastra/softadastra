/*
 * StorePutCommand.cpp
 */

#include "commands/store/StorePutCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/Value.hpp>

namespace softadastra::app::cli::commands::store
{
  namespace store_types = softadastra::store::types;
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;

  namespace
  {
    [[nodiscard]] std::string mutation_status(
        bool created,
        bool updated)
    {
      if (created)
      {
        return "created";
      }

      if (updated)
      {
        return "updated";
      }

      return "unchanged";
    }
  }

  StorePutCommand::StorePutCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StorePutCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.size() < 2)
    {
      ui::err_line(std::cerr, "Missing key or value argument.");
      ui::tip_line(std::cerr, "Usage: store-put <key> <value>");

      return cli_types::CliErrorCode::MissingArgument;
    }

    const std::string &key_arg = command.args[0];
    const std::string &value_arg = command.args[1];

    if (key_arg.empty())
    {
      ui::err_line(std::cerr, "Key cannot be empty.");

      return cli_types::CliErrorCode::InvalidArguments;
    }

    const store_types::Key key{key_arg};
    const store_types::Value value =
        store_types::Value::from_string(value_arg);

    const auto result =
        runtime_.store().put(key, value);

    if (result.is_err())
    {
      ui::err_line(
          std::cerr,
          "Failed to write value into store.");

      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    const auto &apply_result = result.value();

    if (!apply_result.success)
    {
      ui::err_line(
          std::cerr,
          "Store operation was not applied.");

      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    ui::ok_line(std::cout, "Stored value.");

    const std::vector<std::string> headers{
        "Field",
        "Value",
    };

    const std::vector<std::vector<std::string>> rows{
        {
            "key",
            key.value(),
        },
        {
            "version",
            std::to_string(apply_result.version),
        },
        {
            "status",
            mutation_status(
                apply_result.created,
                apply_result.updated),
        },
    };

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::store
