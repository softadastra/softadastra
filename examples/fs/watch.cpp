/*
 * watch.cpp
 */

#include <iostream>
#include <thread>
#include <chrono>

#include <softadastra/fs/Fs.hpp>

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

  auto result = watcher.start(root, [](const events::EventBatch &batch)
                              {
    for (const auto &e : batch.all())
    {
      std::cout << "Event: "
                << types::to_string(e.type) << " "
                << e.current.path.str() << "\n";
    } });

  if (result.is_err())
  {
    std::cerr << "Error: "
              << result.error().message() << "\n";
    return 1;
  }

  std::cout << "Watching... press Ctrl+C to exit\n";

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
