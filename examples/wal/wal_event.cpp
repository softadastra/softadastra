/*
 * wal_event.cpp
 */

#include <filesystem>
#include <iostream>
#include <optional>

#include <softadastra/wal/Wal.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== WAL FILE EVENT EXAMPLE ==\n";

  const std::string path = "event_wal.log";
  std::filesystem::remove(path);

  wal::writer::WalWriter writer{
      wal::core::WalConfig::durable(path)};

  auto path_result = fs::path::Path::from("docs/file.txt");

  if (path_result.is_err())
  {
    std::cerr << "Invalid path: "
              << path_result.error().message()
              << "\n";
    return 1;
  }

  fs::state::FileMetadata metadata{};
  metadata.type = fs::types::FileType::File;
  metadata.size = 128;
  metadata.modified = core::time::Timestamp::now();

  fs::state::FileState state{
      path_result.value(),
      metadata,
      std::nullopt};

  fs::events::FileEvent event{
      fs::types::FileEventType::Created,
      state,
      std::nullopt};

  auto result = writer.append_event(event);

  if (result.is_err())
  {
    std::cerr << "Failed to append event: "
              << result.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Event written with seq=" << result.value() << "\n";

  return 0;
}
