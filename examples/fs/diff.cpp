/*
 * diff.cpp
 */

#include <iostream>
#include <thread>

#include <softadastra/fs/snapshot/SnapshotBuilder.hpp>
#include <softadastra/fs/snapshot/SnapshotDiff.hpp>
#include <softadastra/fs/types/FileEventType.hpp>

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

  auto before_result = snapshot::SnapshotBuilder::build(root);
  if (before_result.is_err())
  {
    std::cerr << "Error building snapshot (before)\n";
    return 1;
  }

  auto before = before_result.value();

  std::cout << "Modify files now...\n";
  std::this_thread::sleep_for(std::chrono::seconds(5));

  auto after_result = snapshot::SnapshotBuilder::build(root);
  if (after_result.is_err())
  {
    std::cerr << "Error building snapshot (after)\n";
    return 1;
  }

  auto after = after_result.value();

  auto diff = snapshot::SnapshotDiff::compute(
      before.all(),
      after.all());

  for (const auto &c : diff)
  {
    std::cout << "Change: ";

    switch (c.type)
    {
    case types::FileEventType::Created:
      std::cout << "Created ";
      break;
    case types::FileEventType::Updated:
      std::cout << "Updated ";
      break;
    case types::FileEventType::Deleted:
      std::cout << "Deleted ";
      break;
    }

    std::cout << c.current.path.str();

    std::cout << std::endl;
  }

  return 0;
}
