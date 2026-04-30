/**
 *
 *  @file NodeInfoCommand.hpp
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
   * @brief Displays local Softadastra node information.
   *
   * NodeInfoCommand reads metadata from the composed runtime and prints a
   * readable summary of the local node.
   *
   * It reports:
   * - node id
   * - display name
   * - hostname
   * - operating system
   * - runtime version
   * - uptime
   * - capabilities count
   *
   * The command is read-only. It must not mutate runtime state.
   */
  class NodeInfoCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a node info command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime to inspect.
     */
    explicit NodeInfoCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the node info command.
     *
     * @param command Parsed CLI command.
     * @return CLI error code.
     */
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::node

#endif // SOFTADASTRA_APPS_CLI_NODE_INFO_COMMAND_HPP
