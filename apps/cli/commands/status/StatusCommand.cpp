/*
 * StatusCommand.cpp
 */

#include "commands/status/StatusCommand.hpp"

#include <iostream>

namespace softadastra::app::cli::commands::status
{
  StatusCommand::StatusCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode StatusCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    const auto &sync_state = runtime_.sync().state();

    std::cout << "Softadastra status\n\n";

    std::cout << "node_id: "
              << runtime_.node_id()
              << "\n";

    std::cout << "node_running: "
              << (runtime_.node_running() ? "yes" : "no")
              << "\n";

    std::cout << "store_entries: "
              << runtime_.store().entries().size()
              << "\n";

    std::cout << "sync_outbox: "
              << sync_state.outbox_size
              << "\n";

    std::cout << "sync_queue: "
              << sync_state.queued_count
              << "\n";

    std::cout << "sync_in_flight: "
              << sync_state.in_flight_count
              << "\n";

    std::cout << "sync_acknowledged: "
              << sync_state.acknowledged_count
              << "\n";

    std::cout << "sync_failed: "
              << sync_state.failed_count
              << "\n";

    std::cout << "transport_running: "
              << (runtime_.transport().running() ? "yes" : "no")
              << "\n";

    std::cout << "transport_peers: "
              << runtime_.transport().peers().size()
              << "\n";

    std::cout << "discovery_running: "
              << (runtime_.discovery().running() ? "yes" : "no")
              << "\n";

    std::cout << "discovery_peers: "
              << runtime_.discovery().peers().size()
              << "\n";

    std::cout << "metadata_running: "
              << (runtime_.metadata().running() ? "yes" : "no")
              << "\n";

    const auto metadata = runtime_.metadata().local();

    if (metadata.has_value())
    {
      std::cout << "hostname: "
                << metadata->runtime.hostname
                << "\n";

      std::cout << "os: "
                << metadata->runtime.os_name
                << "\n";

      std::cout << "version: "
                << metadata->runtime.version
                << "\n";

      std::cout << "uptime_ms: "
                << metadata->runtime.uptime_ms
                << "\n";
    }

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::status
