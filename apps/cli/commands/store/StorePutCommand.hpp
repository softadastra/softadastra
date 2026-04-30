/**
 *
 *  @file StorePutCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_STORE_PUT_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_STORE_PUT_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::store
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  /**
   * @brief Writes a key/value pair into the local Softadastra store.
   *
   * StorePutCommand inserts or replaces one local key/value entry using the
   * composed Softadastra runtime store.
   *
   * It reports:
   * - written key
   * - assigned version
   * - mutation status
   *
   * The command mutates local store state and may trigger WAL-backed
   * persistence depending on the runtime store configuration.
   *
   * Usage:
   * @code
   * store-put <key> <value>
   * @endcode
   */
  class StorePutCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a store put command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime containing the local store.
     */
    explicit StorePutCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the store put command.
     *
     * @param command Parsed CLI command.
     * @return CLI error code.
     */
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::store

#endif // SOFTADASTRA_APPS_CLI_STORE_PUT_COMMAND_HPP
