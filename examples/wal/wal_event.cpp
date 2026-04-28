/*
 * wal_event.cpp
 */

#include <iostream>
#include <filesystem>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/state/FileMetadata.hpp>
#include <softadastra/fs/types/FileType.hpp>

#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== WAL FILE EVENT EXAMPLE ==\n";

  const std::string path = "event_wal.log";
  std::filesystem::remove(path);

  wal::core::WalConfig config;
  config.path = path;
  config.auto_flush = true;

  wal::writer::WalWriter writer(config);

  auto p = fs::path::Path::from("docs/file.txt").value();

  fs::state::FileMetadata meta;
  meta.type = fs::types::FileType::File;

  fs::state::FileState state{p, meta, std::nullopt};

  fs::events::FileEvent event{
      fs::types::FileEventType::Created,
      state,
      std::nullopt};

  wal::core::WalRecord record;
  record.type = wal::types::WalRecordType::Put;
  record.timestamp = 123456;

  record.payload =
      wal::utils::FileEventSerializer::serialize(event);

  auto seq = writer.append(record);

  std::cout << "Event written with seq=" << seq << "\n";

  return 0;
}
