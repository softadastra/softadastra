/*
 * scan.cpp
 */

#include <iostream>

#include <softadastra/fs/Fs.hpp>

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

  auto result = scanner::Scanner::scan(root);

  if (result.is_err())
  {
    std::cerr << "Error: "
              << result.error().message() << "\n";
    return 1;
  }

  const auto &snapshot = result.value();

  for (const auto &[_, file] : snapshot.all())
  {
    std::cout
        << types::to_string(file.metadata.type) << " | "
        << file.path.str() << " | "
        << file.metadata.size << " bytes\n";
  }

  return 0;
}
