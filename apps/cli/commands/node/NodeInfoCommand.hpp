/*
 * NodeInfoCommand.hpp
 */

#ifndef SOFTADASTRA_APPS_CLI_NODE_INFO_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_NODE_INFO_COMMAND_HPP

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
   * @brief Shows local Softadastra node information
   *
   * This command reads metadata from the composed runtime and prints:
   * - node id
   * - display name
   * - hostname
   * - operating system
   * - version
   * - uptime
   * - capabilities count
   */
  class NodeInfoCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit NodeInfoCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the node info command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::node

#endif
