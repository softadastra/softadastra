/**
 *
 *  @file NodeCommand.hpp
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

#ifndef SOFTADASTRA_APPS_CLI_NODE_COMMAND_HPP
#define SOFTADASTRA_APPS_CLI_NODE_COMMAND_HPP

#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/parser/ParsedCommand.hpp>
#include <softadastra/cli/types/CliErrorCode.hpp>

#include "commands/node/NodeInfoCommand.hpp"
#include "commands/node/NodeStartCommand.hpp"
#include "runtime/SoftadastraRuntime.hpp"

namespace softadastra::app::cli::commands::node
{
  namespace cli_command = softadastra::cli::command;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  class NodeCommand final : public cli_command::ICommandHandler
  {
  public:
    explicit NodeCommand(SoftadastraRuntime &runtime);

    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override;

  private:
    SoftadastraRuntime &runtime_;
    NodeInfoCommand info_;
    NodeStartCommand start_;
  };

} // namespace softadastra::app::cli::commands::node

#endif // SOFTADASTRA_APPS_CLI_NODE_COMMAND_HPP
