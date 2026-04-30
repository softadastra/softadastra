/*
 * cli_minimal.cpp
 */

#include <softadastra/cli/cli.hpp>

int main()
{
  softadastra::cli::core::CliConfig config;
  config.app_name = "softadastra-cli";
  config.version = "0.1.0";
  config.interactive = true;
  config.show_banner = true;

  softadastra::cli::CliService cli(config);

  softadastra::cli::CliOptions options;
  options.interactive = true;

  return cli.run(options);
}
