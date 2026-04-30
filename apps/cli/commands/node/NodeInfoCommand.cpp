/*
 * NodeInfoCommand.cpp
 */

#include "commands/node/NodeInfoCommand.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/utils/TableFormatter.hpp>
#include <softadastra/cli/utils/Ui.hpp>

namespace softadastra::app::cli::commands::node
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

  NodeInfoCommand::NodeInfoCommand(SoftadastraRuntime &runtime)
      : runtime_(runtime)
  {
  }

  cli_types::CliErrorCode NodeInfoCommand::handle(
      const cli_parser::ParsedCommand &)
  {
    auto metadata = runtime_.metadata().local();

    if (!metadata.has_value())
    {
      metadata = runtime_.metadata().local_or_refresh();
    }

    if (!metadata.has_value())
    {
      ui::err_line(
          std::cerr,
          "Unable to read local node metadata.");

      return cli_types::CliErrorCode::CommandExecutionFailed;
    }

    ui::section(std::cout, "Softadastra node");

    const std::vector<std::string> headers{
        "Field",
        "Value",
    };

    std::vector<std::vector<std::string>> rows{
        std::vector<std::string>{
            "node_id",
            metadata->identity.node_id,
        },
        std::vector<std::string>{
            "display_name",
            metadata->identity.display_name,
        },
        std::vector<std::string>{
            "hostname",
            metadata->runtime.hostname,
        },
        std::vector<std::string>{
            "os",
            metadata->runtime.os_name,
        },
        std::vector<std::string>{
            "version",
            metadata->runtime.version,
        },
        std::vector<std::string>{
            "started_at",
            std::to_string(metadata->runtime.started_at.millis()),
        },
        std::vector<std::string>{
            "uptime_ms",
            std::to_string(metadata->runtime.uptime_ms()),
        },
        std::vector<std::string>{
            "capabilities",
            std::to_string(metadata->capabilities.values.size()),
        },
        std::vector<std::string>{
            "node_running",
            yes_no(runtime_.node_running()),
        },
    };

    std::cout << "\n"
              << cli_utils::TableFormatter::format(headers, rows);

    return cli_types::CliErrorCode::None;
  }

} // namespace softadastra::app::cli::commands::node
