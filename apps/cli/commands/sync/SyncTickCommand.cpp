/*
 * SyncTickCommand.cpp
 */

#include "commands/sync/SyncTickCommand.hpp"

#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>
#include <softadastra/sync/scheduler/SyncScheduler.hpp>

namespace softadastra::app::cli::commands::sync
{
  namespace sync_scheduler = softadastra::sync::scheduler;
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;

  SyncTickCommand::SyncTickCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode SyncTickCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    sync_scheduler::SyncScheduler scheduler(runtime_.sync());

    const auto tick =
        scheduler.tick(false);

    std::size_t sent_count = 0;

    const auto peers =
        runtime_.transport()
            .peers()
            .connected_peers();

    for (const auto &peer_session : peers)
    {
      if (!peer_session.is_valid() ||
          !peer_session.connected())
      {
        continue;
      }

      sent_count +=
          runtime_.transport().send_sync_batch(
              peer_session.peer,
              tick.batch);
    }

    ui::section(std::cout, "Softadastra sync tick");

    const std::vector<std::string> headers{
        "Metric",
        "Value",
    };

    const std::vector<std::vector<std::string>> rows{
        {
            "retried_count",
            std::to_string(tick.retried_count),
        },
        {
            "batch_size",
            std::to_string(tick.batch.size()),
        },
        {
            "connected_peers",
            std::to_string(peers.size()),
        },
        {
            "sent_count",
            std::to_string(sent_count),
        },
        {
            "pruned_count",
            std::to_string(tick.pruned_count),
        },
    };

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    if (tick.batch.empty())
    {
      ui::info_line(std::cout, "No sync operations ready for delivery.");
    }
    else if (peers.empty())
    {
      ui::warn_line(std::cout, "No connected transport peers available.");
    }
    else if (sent_count == 0)
    {
      ui::warn_line(std::cout, "No sync batch was sent.");
    }
    else
    {
      ui::ok_line(std::cout, "Sync batch delivery attempted.");
    }

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::sync
