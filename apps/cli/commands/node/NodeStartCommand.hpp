/*
 * NodeStartCommand.hpp
 */

#ifndef SOFTADASTRA_APPS_CLI_NODE_START_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_NODE_START_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::node
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  /**
   * @brief Starts the local Softadastra node services
   *
   * Starts:
   * - transport
   * - discovery
   * - metadata
   */
  class NodeStartCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit NodeStartCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the node start command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::node

#endif
