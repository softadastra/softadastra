/*
 * StoreGetCommand.cpp
 */

#include "commands/store/StoreGetCommand.hpp"

#include <iostream>
#include <string>

#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/utils/Serializer.hpp>

namespace softadastra::app::cli::commands::store
{
  namespace store_types = softadastra::store::types;
  namespace store_utils = softadastra::store::utils;

  StoreGetCommand::StoreGetCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StoreGetCommand::handle(
      const cli_parser::ParsedCommand &command)
  {
    if (command.args.empty())
    {
      std::cerr << "Usage: store-get <key>\n";
      return cli_types::CliErrorCode::MissingArgument;
    }

    const std::string &key_arg = command.args[0];

    if (key_arg.empty())
    {
      std::cerr << "Key cannot be empty.\n";
      return cli_types::CliErrorCode::InvalidArguments;
    }

    store_types::Key key;
    key.value = key_arg;

    const auto entry = runtime_.store().get(key);

    if (!entry.has_value())
    {
      std::cerr << "Key not found.\n";
      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    std::cout << "Store entry\n\n";
    std::cout << "key: " << entry->key.value << "\n";
    std::cout << "value: "
              << store_utils::Serializer::to_string(entry->value.data)
              << "\n";
    std::cout << "version: " << entry->version << "\n";
    std::cout << "timestamp: " << entry->timestamp << "\n";

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::store
