/**
 *
 *  @file StoreGetCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_STORE_GET_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_STORE_GET_COMMAND_HPP

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
   * @brief Reads one key from the local Softadastra store.
   *
   * StoreGetCommand reads a single key from the local StoreEngine and prints
   * the stored entry in a readable format.
   *
   * It reports:
   * - key
   * - value
   * - version
   * - timestamp
   *
   * The command is read-only. It must not mutate store state.
   *
   * Usage:
   * @code
   * store-get <key>
   * @endcode
   */
  class StoreGetCommand final : public cli_command::ICommandHandler
  {
  public:
    /**
     * @brief Creates a store get command bound to a runtime instance.
     *
     * @param runtime Softadastra runtime containing the local store.
     */
    explicit StoreGetCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Executes the store get command.
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

#endif // SOFTADASTRA_APPS_CLI_STORE_GET_COMMAND_HPP
