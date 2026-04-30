/*
 * cli_ui_style.cpp
 */

#include <iostream>

#include <softadastra/cli/cli.hpp>

int main()
{
  namespace ui = softadastra::cli::utils::ui;
  namespace style = softadastra::cli::utils::style;

  ui::section(std::cout, "Softadastra CLI");

  ui::ok_line(std::cout, "Runtime initialized");
  ui::info_line(std::cout, "Loading command registry");
  ui::warn_line(std::cout, "Experimental command enabled");
  ui::err_line(std::cout, "Example error output");

  ui::spacer(std::cout);

  ui::kv(std::cout, "app", "softadastra");
  ui::kv(std::cout, "version", "0.1.0");
  ui::kv(std::cout, "mode", "production");

  ui::spacer(std::cout);

  std::cout << style::bold("Strong text") << "\n";
  std::cout << style::dim("Muted text") << "\n";
  std::cout << style::link("https://github.com/softadastra/softadastra") << "\n";

  return 0;
}
