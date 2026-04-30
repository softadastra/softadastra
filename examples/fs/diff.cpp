/*
 * diff.cpp
 */

#include <iostream>
#include <thread>

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

  auto before_result = snapshot::SnapshotBuilder::build(root);
  if (before_result.is_err())
  {
    std::cerr << "Error building snapshot (before)\n";
    return 1;
  }

  auto before = std::move(before_result.value());

  std::cout << "Modify files now...\n";
  std::this_thread::sleep_for(std::chrono::seconds(5));

  auto after_result = snapshot::SnapshotBuilder::build(root);
  if (after_result.is_err())
  {
    std::cerr << "Error building snapshot (after)\n";
    return 1;
  }

  auto after = std::move(after_result.value());

  auto events = snapshot::SnapshotDiff::compute(
      before.all(),
      after.all());

  for (const auto &e : events)
  {
    std::cout << "Change: "
              << types::to_string(e.type) << " ";

    // Deleted → current = previous
    if (e.is_deleted())
    {
      std::cout << e.current.path.str();
    }
    else
    {
      std::cout << e.current.path.str();
    }

    std::cout << "\n";
  }

  return 0;
}
