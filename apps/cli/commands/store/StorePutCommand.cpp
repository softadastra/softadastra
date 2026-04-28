/*
 * StorePutCommand.cpp
 */

#include "commands/store/StorePutCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/Value.hpp>
#include <softadastra/store/utils/Serializer.hpp>

namespace softadastra::app::cli::commands::store
{
  namespace store_types = softadastra::store::types;
  namespace store_utils = softadastra::store::utils;

  StorePutCommand::StorePutCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StorePutCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.size() < 2)
    {
      std::cerr << "Usage: store-put <key> <value>\n";
      return cli_types::CliErrorCode::MissingArgument;
    }

    const std::string &key_arg = command.args[0];
    const std::string &value_arg = command.args[1];

    if (key_arg.empty())
    {
      std::cerr << "Key cannot be empty.\n";
      return cli_types::CliErrorCode::InvalidArguments;
    }

    store_types::Key key;
    key.value = key_arg;

    store_types::Value value;
    value.data = store_utils::Serializer::to_bytes(value_arg);

    const auto result = runtime_.store().put(key, value);

    if (!result.success)
    {
      std::cerr << "Failed to write value into store.\n";
      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    std::cout << "Stored value.\n";
    std::cout << "key: " << key.value << "\n";
    std::cout << "version: " << result.version << "\n";

    if (result.created)
    {
      std::cout << "status: created\n";
    }
    else if (result.updated)
    {
      std::cout << "status: updated\n";
    }
    else
    {
      std::cout << "status: unchanged\n";
    }

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::store
