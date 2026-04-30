/**
 *
 *  @file SyncTickCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_SYNC_TICK_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_SYNC_TICK_COMMAND_HPP

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
   * @brief Executes one manual sync scheduler cycle.
   *
   * SyncTickCommand manually advances the local sync pipeline once.
   *
   * It is useful for:
   * - debugging sync behavior
   * - manually flushing pending operations
   * - testing retry behavior
   * - observing transport delivery
   *
   * The command may mutate sync and transport state by retrying expired
   * operations, collecting ready batches, and sending them to connected peers.
   *
   * Usage:
   * @code
   * sync-tick
   * @endcode
   */
  class SyncTickCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a sync tick command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime containing sync and transport state.
     */
    explicit SyncTickCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the sync tick command.
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

#endif // SOFTADASTRA_APPS_CLI_SYNC_TICK_COMMAND_HPP
