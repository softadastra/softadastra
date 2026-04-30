/**
 *
 *  @file SyncStatusCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_SYNC_STATUS_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_SYNC_STATUS_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::sync
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  /**
   * @brief Displays the current Softadastra sync engine state.
   *
   * SyncStatusCommand inspects the local SyncEngine state and prints a readable
   * operational summary for developers and operators.
   *
   * It reports:
   * - outbox size
   * - queued operations
   * - in-flight operations
   * - acknowledged operations
   * - failed operations
   *
   * The command is read-only. It must not mutate sync state.
   *
   * Usage:
   * @code
   * sync-status
   * @endcode
   */
  class SyncStatusCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a sync status command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime containing the local sync engine.
     */
    explicit SyncStatusCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the sync status command.
     *
     * @param command Parsed CLI command.
     * @return CLI error code.
     */
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::sync

#endif // SOFTADASTRA_APPS_CLI_SYNC_STATUS_COMMAND_HPP
