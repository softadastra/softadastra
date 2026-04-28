/*
 * PeersCommand.hpp
 */

#ifndef SOFTADASTRA_APPS_CLI_PEERS_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_PEERS_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::peers
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  /**
   * @brief Lists peers known by discovery and transport
   *
   * Usage:
   *   peers
   */
  class PeersCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit PeersCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the peers command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::peers

#endif
