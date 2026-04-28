/*
 * SyncStatusCommand.cpp
 */

#include "commands/sync/SyncStatusCommand.hpp"

#include <iostream>

namespace softadastra::app::cli::commands::sync
{
  SyncStatusCommand::SyncStatusCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode SyncStatusCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    const auto &state = runtime_.sync().state();

    std::cout << "Softadastra sync status\n\n";

    std::cout << "node_id: "
              << runtime_.node_id()
              << "\n";

    std::cout << "outbox_size: "
              << state.outbox_size
              << "\n";

    std::cout << "queued_count: "
              << state.queued_count
              << "\n";

    std::cout << "in_flight_count: "
              << state.in_flight_count
              << "\n";

    std::cout << "acknowledged_count: "
              << state.acknowledged_count
              << "\n";

    std::cout << "failed_count: "
              << state.failed_count
              << "\n";

    std::cout << "last_submitted_version: "
              << state.last_submitted_version
              << "\n";

    std::cout << "last_applied_remote_version: "
              << state.last_applied_remote_version
              << "\n";

    std::cout << "total_retries: "
              << state.total_retries
              << "\n";

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::sync
