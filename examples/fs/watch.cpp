/*
 * watch.cpp
 */

#include <iostream>
#include <thread>
#include <chrono>

#include <softadastra/fs/watcher/Watcher.hpp>
#include <softadastra/fs/types/FileEventType.hpp>

using namespace softadastra::fs;

int main()
{
  auto root_result = path::Path::from("./watched");

  if (root_result.is_err())
  {
    std::cerr << "Invalid path\n";
    return 1;
  }

  auto root = root_result.value();

  watcher::Watcher watcher;

  watcher.start(root, [](const events::EventBatch &batch)
                {
    for (const auto &e : batch.all())
    {
      std::cout << "Event: ";

      switch (e.type)
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

      std::cout << e.current.path.str();

      std::cout << std::endl;
    } });

  std::cout << "Watching... press Ctrl+C to exit\n";

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
