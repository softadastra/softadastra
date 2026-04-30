/*
 * cli_table_formatter.cpp
 */

#include <iostream>
#include <string>
#include <vector>

#include <softadastra/cli/cli.hpp>

int main()
{
  const std::vector<std::string> headers{
      "Command",
      "Type",
      "Description",
  };

  const std::vector<std::vector<std::string>> rows{
      {"help", "info", "Show help information"},
      {"version", "info", "Show CLI version"},
      {"exit", "system", "Exit the CLI session"},
      {"node-start", "admin", "Start a Softadastra node"},
  };

  std::cout << softadastra::cli::utils::TableFormatter::format(
      headers,
      rows);

  return 0;
}
