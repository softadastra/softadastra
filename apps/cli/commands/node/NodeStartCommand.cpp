/*
 * NodeStartCommand.cpp
 */

#include "commands/node/NodeStartCommand.hpp"

#include <iostream>

#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::node
{
  namespace ui = softadastra::cli::utils::ui;

  NodeStartCommand::NodeStartCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode NodeStartCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    if (runtime_.node_running())
    {
      ui::warn_line(
          std::cout,
          "Softadastra node is already running.");

      ui::kv(
          std::cout,
          "node_id",
          runtime_.node_id());

      return cli_types::CliErrorCode::None;
    }

    ui::section(std::cout, "Starting Softadastra node");

    if (!runtime_.start_node())
    {
      ui::err_line(
          std::cerr,
          "Failed to start Softadastra node.");

      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    ui::ok_line(
        std::cout,
        "Softadastra node services started for this CLI session.");

    ui::kv(
        std::cout,
        "node_id",
        runtime_.node_id());

    ui::kv(
        std::cout,
        "transport",
        runtime_.transport().running() ? "running" : "stopped");

    ui::kv(
        std::cout,
        "discovery",
        runtime_.discovery().running() ? "running" : "stopped");

    ui::kv(
        std::cout,
        "metadata",
        runtime_.metadata().running() ? "running" : "stopped");

    ui::tip_line(
        std::cout,
        "Use the Softadastra node app for a long-running daemon.");

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::node
