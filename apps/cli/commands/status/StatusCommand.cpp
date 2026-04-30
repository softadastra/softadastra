/*
 * StatusCommand.cpp
 */

#include "commands/status/StatusCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::status
{
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;

  namespace
  {
    [[nodiscard]] std::string yes_no(bool value)
    {
      return value ? "yes" : "no";
    }
  }

  StatusCommand::StatusCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StatusCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    const auto &sync_state = runtime_.sync().state();

    ui::section(std::cout, "Softadastra status");

    const std::vector<std::string> headers{
        "Component",
        "Metric",
        "Value",
    };

    std::vector<std::vector<std::string>> rows{
        {
            "node",
            "id",
            runtime_.node_id(),
        },
        {
            "node",
            "running",
            yes_no(runtime_.node_running()),
        },
        {
            "store",
            "entries",
            std::to_string(runtime_.store().entries().size()),
        },
        {
            "sync",
            "outbox",
            std::to_string(sync_state.outbox_size),
        },
        {
            "sync",
            "queued",
            std::to_string(sync_state.queued_count),
        },
        {
            "sync",
            "in_flight",
            std::to_string(sync_state.in_flight_count),
        },
        {
            "sync",
            "acknowledged",
            std::to_string(sync_state.acknowledged_count),
        },
        {
            "sync",
            "failed",
            std::to_string(sync_state.failed_count),
        },
        {
            "transport",
            "running",
            yes_no(runtime_.transport().running()),
        },
        {
            "transport",
            "peers",
            std::to_string(runtime_.transport().peers().size()),
        },
        {
            "discovery",
            "running",
            yes_no(runtime_.discovery().running()),
        },
        {
            "discovery",
            "peers",
            std::to_string(runtime_.discovery().peers().size()),
        },
        {
            "metadata",
            "running",
            yes_no(runtime_.metadata().running()),
        },
    };

    const auto metadata = runtime_.metadata().local();

    if (metadata.has_value())
    {
      rows.push_back({
          "metadata",
          "hostname",
          metadata->runtime.hostname,
      });

      rows.push_back({
          "metadata",
          "os",
          metadata->runtime.os_name,
      });

      rows.push_back({
          "metadata",
          "version",
          metadata->runtime.version,
      });

      rows.push_back({
          "metadata",
          "uptime_ms",
          std::to_string(metadata->runtime.uptime_ms()),
      });
    }

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::status
