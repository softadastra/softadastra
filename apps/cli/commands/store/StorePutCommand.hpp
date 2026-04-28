/*
 * StorePutCommand.hpp
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
   * @brief Writes a key/value pair into the local Softadastra store
   *
   * Usage:
   *   store-put <key> <value>
   */
  class StorePutCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit StorePutCommand(SoftadastraRuntime &runtime);

    /**
     * @brief Execute the store put command
     */
    cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
  };

} // namespace softadastra::app::cli::commands::store

#endif
