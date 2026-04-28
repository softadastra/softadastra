/*
 * snapshot.cpp
 */

#include <iostream>
#include <softadastra/fs/snapshot/SnapshotBuilder.hpp>

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

  auto result = snapshot::SnapshotBuilder::build(root);

  if (result.is_err())
  {
    std::cerr << "Error: " << result.error().message() << std::endl;
    return 1;
  }

  const auto &snap = result.value();

  for (const auto &[_, state] : snap.all())
  {
    std::cout << state.path.str() << std::endl;
  }

  return 0;
}
