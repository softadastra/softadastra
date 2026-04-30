/**
 *
 *  @file NodeStartCommand.hpp
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
   * @brief Starts the local Softadastra node services.
   *
   * NodeStartCommand starts the composed local runtime services required for a
   * Softadastra node to participate in the local-first infrastructure.
   *
   * It starts:
   * - transport service
   * - discovery service
   * - metadata service
   *
   * The command is operational and may mutate runtime state by transitioning
   * services from stopped to running.
   *
   * Usage:
   * @code
   * node-start
   * @endcode
   */
  class NodeStartCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a node start command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime to start.
     */
    explicit NodeStartCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the node start command.
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

#endif // SOFTADASTRA_APPS_CLI_NODE_START_COMMAND_HPP
