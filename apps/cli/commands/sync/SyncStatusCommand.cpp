/*
 * SyncStatusCommand.cpp
 */

#include "commands/sync/SyncStatusCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::sync
{
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;

  SyncStatusCommand::SyncStatusCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode SyncStatusCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    const auto &state = runtime_.sync().state();

    ui::section(std::cout, "Softadastra sync status");

    const std::vector<std::string> headers{
        "Metric",
        "Value",
    };

    const std::vector<std::vector<std::string>> rows{
        {
            "node_id",
            runtime_.node_id(),
        },
        {
            "outbox_size",
            std::to_string(state.outbox_size),
        },
        {
            "queued_count",
            std::to_string(state.queued_count),
        },
        {
            "in_flight_count",
            std::to_string(state.in_flight_count),
        },
        {
            "acknowledged_count",
            std::to_string(state.acknowledged_count),
        },
        {
            "failed_count",
            std::to_string(state.failed_count),
        },
        {
            "last_submitted_version",
            std::to_string(state.last_submitted_version),
        },
        {
            "last_applied_remote_version",
            std::to_string(state.last_applied_remote_version),
        },
        {
            "total_retries",
            std::to_string(state.total_retries),
        },
    };

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::sync
