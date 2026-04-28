/*
 * main.cpp
 */

#include <string>
#include <vector>

#include <softadastra/cli/cli.hpp>
#include "runtime/SoftadastraRuntime.hpp"

int main(int argc, char **argv)
{
  softadastra::cli::core::CliConfig config;
  config.app_name = "softadastra";
  config.version = "0.1.0";
  config.interactive = true;
  config.show_banner = true;

  softadastra::app::cli::SoftadastraRuntime runtime(config);

  if (!runtime.valid())
  {
    return 1;
  }

  softadastra::cli::CliOptions options;
  options.interactive = true;

  for (int i = 1; i < argc; ++i)
  {
    options.args.emplace_back(argv[i]);
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

  return runtime.cli().run(options);
}
