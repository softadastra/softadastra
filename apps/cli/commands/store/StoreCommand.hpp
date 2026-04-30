/**
 *
 *  @file StoreCommand.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra CLI App
 *
 */

#ifndef SOFTADASTRA_APPS_CLI_STORE_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_STORE_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "commands/store/StoreGetCommand.hpp"
#include "commands/store/StorePutCommand.hpp"
#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::store
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  class StoreCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit StoreCommand(SoftadastraRuntime &runtime);

    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
    StorePutCommand put_;
    StoreGetCommand get_;
  };

} // namespace softadastra::app::cli::commands::store

#endif // SOFTADASTRA_APPS_CLI_STORE_COMMAND_HPP
