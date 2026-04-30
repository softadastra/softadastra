/**
 *
 *  @file SyncCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_SYNC_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_SYNC_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "commands/sync/SyncStatusCommand.hpp"
#include "commands/sync/SyncTickCommand.hpp"
#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::sync
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  class SyncCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit SyncCommand(SoftadastraRuntime &runtime);

    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
    SyncStatusCommand status_;
    SyncTickCommand tick_;
  };

} // namespace softadastra::app::cli::commands::sync

#endif // SOFTADASTRA_APPS_CLI_SYNC_COMMAND_HPP
