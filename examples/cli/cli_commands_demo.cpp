/*
 * cli_commands_demo.cpp
 */

#include <iostream>
#include <memory>

#include <softadastra/cli/cli.hpp>
#include <softadastra/cli/command/CliCommand.hpp>
#include <softadastra/cli/command/ICommandHandler.hpp>
#include <softadastra/cli/core/CliConfig.hpp>
#include <softadastra/cli/core/CliContext.hpp>
#include <softadastra/cli/engine/CliEngine.hpp>

namespace
{
  namespace cli = softadastra::cli;
  namespace cli_command = softadastra::cli::command;
  namespace cli_core = softadastra::cli::core;
  namespace cli_engine = softadastra::cli::engine;
  namespace cli_parser = softadastra::cli::parser;
  namespace cli_types = softadastra::cli::types;

  class HelloCommandHandler : public cli_command::ICommandHandler
  {
  public:
    cli_types::CliErrorCode handle(const cli_parser::ParsedCommand &command) override
    {
      std::cout << "Hello from custom command";

      if (!command.args.empty())
      {
        std::cout << ", " << command.args.front();
      }

      std::cout << "!\n";
      return cli_types::CliErrorCode::None;
    }
  };

  class SumCommandHandler : public cli_command::ICommandHandler
  {
  public:
    cli_types::CliErrorCode handle(const cli_parser::ParsedCommand &command) override
    {
      if (command.args.size() < 2)
      {
        std::cerr << "sum requires at least 2 integer arguments\n";
        return cli_types::CliErrorCode::InvalidArguments;
      }

      long long total = 0;

      for (const auto &arg : command.args)
      {
        try
        {
          total += std::stoll(arg);
        }
        catch (...)
        {
          std::cerr << "invalid integer: " << arg << '\n';
          return cli_types::CliErrorCode::InvalidArguments;
        }
      }

      std::cout << "sum = " << total << '\n';
      return cli_types::CliErrorCode::None;
    }
  };

  void register_custom_commands(cli_command::CommandRegistry &registry)
  {
    registry.register_command(
        cli_command::CliCommand{
            "hello",
            "Print a greeting message",
            "hello [name]",
            cli_types::CliCommandType::Custom,
            {"hi"}},
        std::make_shared<HelloCommandHandler>());

    registry.register_command(
        cli_command::CliCommand{
            "sum",
            "Sum integer arguments",
            "sum <a> <b> [c] ...",
            cli_types::CliCommandType::Custom,
            {}},
        std::make_shared<SumCommandHandler>());
  }
}

int main()
{
  cli_core::CliConfig config;
  config.app_name = "softadastra-cli-demo";
  config.version = "0.1.0";
  config.interactive = false;
  config.show_banner = false;

  cli_command::CommandRegistry registry;
  register_custom_commands(registry);

  cli_core::CliContext context;
  context.config = &config;
  context.registry = &registry;

  cli_engine::CliEngine engine(context);

  if (!engine.start())
  {
    std::cerr << "failed to start CLI engine\n";
    return 1;
  }

  std::cout << "Running demo commands...\n";

  auto result = engine.execute("hello");
  if (result != cli_types::CliErrorCode::None)
  {
    std::cerr << "hello command failed\n";
    return 1;
  }

  result = engine.execute("hello Gaspard");
  if (result != cli_types::CliErrorCode::None)
  {
    std::cerr << "hello Gaspard command failed\n";
    return 1;
  }

  result = engine.execute("sum 10 20 30");
  if (result != cli_types::CliErrorCode::None)
  {
    std::cerr << "sum command failed\n";
    return 1;
  }

  engine.stop();
  return 0;
}
