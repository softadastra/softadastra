/*
 * cli_arg_parser.cpp
 */

#include <iostream>
#include <string>

#include <softadastra/cli/cli.hpp>

namespace cli_parser = softadastra::cli::parser;
namespace cli_types = softadastra::cli::types;

namespace
{
  std::string value_to_string(const cli_types::OptionValue &value)
  {
    if (std::holds_alternative<std::monostate>(value))
    {
      return "<empty>";
    }

    if (std::holds_alternative<bool>(value))
    {
      return std::get<bool>(value) ? "true" : "false";
    }

    if (std::holds_alternative<std::int64_t>(value))
    {
      return std::to_string(std::get<std::int64_t>(value));
    }

    if (std::holds_alternative<double>(value))
    {
      return std::to_string(std::get<double>(value));
    }

    if (std::holds_alternative<std::string>(value))
    {
      return std::get<std::string>(value);
    }

    return "<unknown>";
  }
}

int main()
{
  const std::string input =
      R"(deploy app --host=localhost --port 8080 --verbose --ratio=0.75)";

  const auto tokens = cli_parser::Tokenizer::tokenize(input);
  const cli_parser::CommandLine line{tokens};
  const auto parsed = cli_parser::ArgParser::parse(line);

  std::cout << "Command: " << parsed.name << "\n\n";

  std::cout << "Arguments:\n";
  for (const auto &arg : parsed.args)
  {
    std::cout << "  - " << arg << "\n";
  }

  std::cout << "\nOptions:\n";
  for (const auto &[key, value] : parsed.options)
  {
    std::cout << "  " << key
              << " = "
              << value_to_string(value)
              << " ("
              << cli_types::option_value_type(value)
              << ")\n";
  }

  return 0;
}
