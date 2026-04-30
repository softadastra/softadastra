/*
 * main.cpp
 */

#include <string>
#include <vector>

#include <softadastra/cli/cli.hpp>

#include "runtime/SoftadastraRuntime.hpp"

namespace
{
  [[nodiscard]] softadastra::cli::CliOptions make_cli_options(
      int argc,
      char **argv)
  {
    softadastra::cli::CliOptions options;
    options.interactive = true;

    for (int i = 1; i < argc; ++i)
    {
      if (argv[i] != nullptr)
      {
        options.args.emplace_back(argv[i]);
      }
    }

    if (!options.args.empty())
    {
      options.interactive = false;

      std::string command;

      for (const auto &arg : options.args)
      {
        if (!command.empty())
        {
          command += " ";
        }

        command += arg;
      }

      options.command = std::move(command);
    }

    return options;
  }
}

int main(int argc, char **argv)
{
  softadastra::cli::core::CliConfig config;

  config.app_name = "softadastra";
  config.version = "0.1.0";
  config.interactive = argc <= 1;
  config.show_banner = argc <= 1;
  config.color_output = true;
  config.verbose = false;
  config.strict_mode = true;

  softadastra::app::cli::SoftadastraRuntime runtime{config};

  if (!runtime.valid())
  {
    return 1;
  }

  const auto options = make_cli_options(argc, argv);

  if (!options.valid())
  {
    return 1;
  }

  return runtime.cli().run(options);
}
