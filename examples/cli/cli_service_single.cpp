/*
 * cli_service_single.cpp
 */

#include <softadastra/cli/cli.hpp>

int main()
{
  softadastra::cli::core::CliConfig config;
  config.app_name = "softadastra";
  config.version = "0.1.0";
  config.interactive = false;
  config.show_banner = false;

  softadastra::cli::CliService service{config};

  return service.run(
      softadastra::cli::CliOptions::single_command("help"));
}
