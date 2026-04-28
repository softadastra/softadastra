/*
 * SyncTickCommand.cpp
 */

#include "commands/sync/SyncTickCommand.hpp"

#include <iostream>

#include <softadastra/sync/scheduler/SyncScheduler.hpp>

namespace softadastra::app::cli::commands::sync
{
  namespace sync_scheduler = softadastra::sync::scheduler;

  SyncTickCommand::SyncTickCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode SyncTickCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    sync_scheduler::SyncScheduler scheduler(runtime_.sync());

    const auto tick = scheduler.tick(false);

    std::size_t sent_count = 0;
    const auto peers = runtime_.transport().peers().connected_peers();

    for (const auto &peer_session : peers)
    {
      if (!peer_session.valid() || !peer_session.connected())
      {
        continue;
      }

      sent_count += runtime_.transport().send_sync_batch(
          peer_session.peer,
          tick.batch);
    }

    std::cout << "Softadastra sync tick\n\n";
    std::cout << "retried_count: " << tick.retried_count << "\n";
    std::cout << "batch_size: " << tick.batch.size() << "\n";
    std::cout << "connected_peers: " << peers.size() << "\n";
    std::cout << "sent_count: " << sent_count << "\n";
    std::cout << "pruned_count: " << tick.pruned_count << "\n";

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::sync
