/*
 * PeersCommand.cpp
 */

#include "commands/peers/PeersCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>
#include <softadastra/transport/types/PeerState.hpp>

namespace softadastra::app::cli::commands::peers
{
  namespace cli_utils = softadastra::cli::utils;
  namespace ui = softadastra::cli::utils::ui;
  namespace transport_types = softadastra::transport::types;

  namespace
  {
    [[nodiscard]] std::string yes_no(bool value)
    {
      return value ? "yes" : "no";
    }

    template <typename Timestamp>
    [[nodiscard]] std::string timestamp_to_string(const Timestamp &timestamp)
    {
      return std::to_string(timestamp.millis());
    }
  }

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

    ui::section(std::cout, "Softadastra peers");

    ui::kv(
        std::cout,
        "discovery",
        yes_no(runtime_.discovery().running()));

    ui::kv(
        std::cout,
        "transport",
        yes_no(runtime_.transport().running()));

    std::cout << "\n";

    {
      ui::section(std::cout, "Discovery peers");

      const std::vector<std::string> headers{
          "Node",
          "Host",
          "Port",
          "Last seen",
      };

      std::vector<std::vector<std::string>> rows;
      rows.reserve(discovery_peers.size());

      for (const auto &peer : discovery_peers)
      {
        rows.push_back(std::vector<std::string>{
            peer.node_id,
            peer.host,
            std::to_string(peer.port),
            timestamp_to_string(peer.last_seen_at),
        });
      }

      if (rows.empty())
      {
        ui::info_line(std::cout, "No discovery peers found.");
      }
      else
      {
        std::cout << cli_utils::TableFormatter::format(headers, rows);
      }
    }

    std::cout << "\n";

    {
      ui::section(std::cout, "Transport peers");

      const std::vector<std::string> headers{
          "Node",
          "Host",
          "Port",
          "State",
          "Connected",
          "Last seen",
          "Errors",
      };

      std::vector<std::vector<std::string>> rows;
      rows.reserve(transport_peers.size());

      for (const auto &session : transport_peers)
      {
        rows.push_back(std::vector<std::string>{
            session.peer.node_id,
            session.peer.host,
            std::to_string(session.peer.port),
            std::string(transport_types::to_string(session.state)),
            yes_no(session.connected()),
            timestamp_to_string(session.last_seen_at),
            std::to_string(session.error_count),
        });
      }

      if (rows.empty())
      {
        ui::info_line(std::cout, "No transport peers found.");
      }
      else
      {
        std::cout << cli_utils::TableFormatter::format(headers, rows);
      }
    }

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::peers
