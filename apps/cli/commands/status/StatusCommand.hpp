/*
 * StatusCommand.hpp
 */

#ifndef SOFTADASTRA_APPS_CLI_STATUS_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_STATUS_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::status
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  /**
   * @brief Shows the current Softadastra runtime status
   *
   * This command inspects the composed runtime:
   * - node identity
   * - store state
   * - sync state
   * - transport peers
   * - discovery peers
   * - metadata state
   */
  class StatusCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit StatusCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the status command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::status

#endif
