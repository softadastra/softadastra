/*
 * SyncTickCommand.hpp
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
   * @brief Executes one manual sync scheduler cycle
   *
   * Usage:
   *   sync-tick
   *
   * This command:
   * - retries expired sync operations
   * - collects the next batch ready for transport
   * - sends the batch to connected transport peers
   */
  class SyncTickCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit SyncTickCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the sync tick command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::sync

#endif
