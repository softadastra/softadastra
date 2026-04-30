/**
 *
 *  @file PeersCommand.hpp
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
   * @brief Lists peers known by the local Softadastra runtime.
   *
   * PeersCommand inspects both discovery and transport peer state and prints a
   * readable peer summary for operators.
   *
   * It reports:
   * - discovered peers
   * - transport peers
   * - peer node ids
   * - host and port information when available
   * - peer state when available
   *
   * The command is read-only. It must not mutate runtime state.
   *
   * Usage:
   * @code
   * peers
   * @endcode
   */
  class PeersCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a peers command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime to inspect.
     */
    explicit PeersCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the peers command.
     *
     * @param command Parsed CLI command.
     * @return CLI error code.
     */
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::peers

#endif // SOFTADASTRA_APPS_CLI_PEERS_COMMAND_HPP
