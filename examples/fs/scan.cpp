/*
 * scan.cpp
 */

#include <iostream>
#include <softadastra/fs/scanner/Scanner.hpp>

using namespace softadastra::fs;

int main()
{
  auto root_result = path::Path::from("./data");

  if (root_result.is_err())
  {
    std::cerr << "Invalid path\n";
    return 1;
  }

  auto root = root_result.value();

  scanner::Scanner scanner;

  auto result = scanner.scan(root);

  if (result.is_err())
  {
    std::cerr << "Error\n";
    return 1;
  }

  const auto &snapshot = result.value();

  for (const auto &[path, file] : snapshot.all())
  {
    std::cout << file.path.str() << std::endl;
  }

  return 0;
}
