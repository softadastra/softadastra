/*
 * cli_registry.cpp
 */

#include <iostream>
#include <memory>

#include <softadastra/cli/cli.hpp>

namespace cli_command = softadastra::cli::command;
namespace cli_parser = softadastra::cli::parser;
namespace cli_types = softadastra::cli::types;

namespace
{
  class PingHandler final : public cli_command::ICommandHandler
  {
  public:
    [[nodiscard]] cli_types::CliErrorCode handle(
        const cli_parser::ParsedCommand &) override
    {
      std::cout << "pong\n";
      return cli_types::CliErrorCode::None;
    }
  };
}

int main()
{
  cli_command::CommandRegistry registry;

  cli_command::CliCommand command{
      "ping",
      "Check that the CLI registry works",
      "ping",
      cli_types::CliCommandType::Diagnostic,
      {"p"},
      {},
  };

  registry.register_command(
      command,
      std::make_shared<PingHandler>());

  std::cout << "Registry size: " << registry.size() << "\n";
  std::cout << "exists ping: " << (registry.exists("ping") ? "yes" : "no") << "\n";
  std::cout << "exists p: " << (registry.exists("p") ? "yes" : "no") << "\n";

  const auto handler = registry.get_handler("p");

  if (handler != nullptr)
  {
    cli_parser::ParsedCommand parsed;
    parsed.name = "p";

    return static_cast<int>(handler->handle(parsed));
  }

  return 1;
}
