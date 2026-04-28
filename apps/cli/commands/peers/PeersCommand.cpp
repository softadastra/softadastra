/*
 * PeersCommand.cpp
 */

#include "commands/peers/PeersCommand.hpp"

#include <iostream>

namespace softadastra::app::cli::commands::peers
{
  PeersCommand::PeersCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode PeersCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    if (runtime_.discovery().running())
    {
      runtime_.discovery().poll_many(32);
    }

    if (runtime_.transport().running())
    {
      runtime_.transport().poll_many(32);
    }

    const auto discovery_peers = runtime_.discovery().peers();
    const auto transport_peers = runtime_.transport().peers().all();

    std::cout << "Softadastra peers\n\n";

    std::cout << "discovery_running: "
              << (runtime_.discovery().running() ? "yes" : "no")
              << "\n";

    std::cout << "transport_running: "
              << (runtime_.transport().running() ? "yes" : "no")
              << "\n\n";

    std::cout << "Discovery peers: "
              << discovery_peers.size()
              << "\n";

    for (const auto &peer : discovery_peers)
    {
      std::cout << "- "
                << peer.node_id
                << " "
                << peer.host
                << ":"
                << peer.port
                << " last_seen_at="
                << peer.last_seen_at
                << "\n";
    }

    std::cout << "\nTransport peers: "
              << transport_peers.size()
              << "\n";

    for (const auto &session : transport_peers)
    {
      std::cout << "- "
                << session.peer.node_id
                << " "
                << session.peer.host
                << ":"
                << session.peer.port
                << " connected="
                << (session.connected() ? "yes" : "no")
                << " last_seen_at="
                << session.last_seen_at
                << " errors="
                << session.error_count
                << "\n";
    }

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::peers
