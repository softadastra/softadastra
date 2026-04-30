/**
 *
 *  @file StatusCommand.hpp
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
   * @brief Displays the current Softadastra runtime status.
   *
   * StatusCommand inspects the composed local runtime and prints a readable
   * operational summary for developers and operators.
   *
   * It reports:
   * - node identity
   * - runtime health
   * - store state
   * - sync state
   * - transport state
   * - discovery state
   * - metadata state
   *
   * The command is read-only. It must not mutate runtime state.
   */
  class StatusCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a status command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime to inspect.
     */
    explicit StatusCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the status command.
     *
     * @param command Parsed CLI command.
     * @return CLI error code.
     */
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::status

#endif // SOFTADASTRA_APPS_CLI_STATUS_COMMAND_HPP
