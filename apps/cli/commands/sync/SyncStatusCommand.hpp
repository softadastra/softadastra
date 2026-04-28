/*
 * SyncStatusCommand.hpp
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
   * @brief Shows the current Softadastra sync engine state
   *
   * Usage:
   *   sync-status
   */
  class SyncStatusCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit SyncStatusCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the sync status command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::sync

#endif
