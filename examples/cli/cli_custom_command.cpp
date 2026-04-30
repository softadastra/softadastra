/*
 * cli_custom_command.cpp
 */

#include <iostream>
#include <memory>

#include <softadastra/cli/cli.hpp>

namespace cli_command = softadastra::cli::command;
namespace cli_core = softadastra::cli::core;
namespace cli_parser = softadastra::cli::parser;
namespace cli_types = softadastra::cli::types;

namespace
{
  class StatusHandler final : public cli_command::ICommandHandler
  {
  public:
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &command) override
    {
      std::cout << "Softadastra status\n";
      std::cout << "  command: " << command.name << "\n";
      std::cout << "  state: healthy\n";
      return cli_types::CliErrorCode::None;
    }
  };
}

int main()
{
  cli_core::CliConfig config;
  config.app_name = "softadastra";
  config.version = "0.1.0";
  config.interactive = false;
  config.show_banner = false;

  softadastra::cli::CliService service{config};

  service.register_command(
      cli_command::CliCommand{
          "status",
          "Show Softadastra runtime status",
          "status",
          cli_types::CliCommandType::Info,
          {"st"},
          {},
      },
      std::make_shared<StatusHandler>());

  return service.run(
      softadastra::cli::CliOptions::single_command("status"));
}
