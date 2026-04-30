/*
 * cli_tokenizer.cpp
 */

#include <iostream>

#include <softadastra/cli/cli.hpp>

int main()
{
  const std::string input =
      R"(store-put name "Softadastra Runtime" --durable true --port=8080)";

  const auto tokens =
      softadastra::cli::parser::Tokenizer::tokenize(input);

  std::cout << "Input:\n";
  std::cout << "  " << input << "\n\n";

  std::cout << "Tokens:\n";

  for (std::size_t i = 0; i < tokens.size(); ++i)
  {
    std::cout << "  [" << i << "] " << tokens[i] << "\n";
  }

  return 0;
}
