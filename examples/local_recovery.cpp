/*
 * local_recovery.cpp
 */

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <softadastra/core/time/Timestamp.hpp>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/snapshot/SnapshotBuilder.hpp>
#include <softadastra/fs/snapshot/SnapshotDiff.hpp>
#include <softadastra/fs/types/FileEventType.hpp>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/Value.hpp>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

namespace
{
  using SnapshotMap = decltype(fs::snapshot::SnapshotBuilder::build(
                                   fs::path::Path::from(".").value())
                                   .value()
                                   .all());

  struct ChangeInfo
  {
    fs::events::FileEvent event;
    std::string key;
  };

  fs::types::FileEventType to_event_type(const fs::types::FileEventType type)
  {
    return type;
  }

  wal::types::WalRecordType to_wal_record_type(const fs::types::FileEventType type)
  {
    switch (type)
    {
    case fs::types::FileEventType::Created:
      return wal::types::WalRecordType::Put;

    case fs::types::FileEventType::Updated:
      return wal::types::WalRecordType::Update;

    case fs::types::FileEventType::Deleted:
      return wal::types::WalRecordType::Delete;
    }

    return wal::types::WalRecordType::Put;
  }

  std::string event_type_to_string(const fs::types::FileEventType type)
  {
    switch (type)
    {
    case fs::types::FileEventType::Created:
      return "Created";
    case fs::types::FileEventType::Updated:
      return "Updated";
    case fs::types::FileEventType::Deleted:
      return "Deleted";
    }

    return "Unknown";
  }

  std::string extract_key_from_event(const fs::events::FileEvent &event)
  {
    return event.current.path.str();
  }

  std::vector<ChangeInfo> build_changes(
      const SnapshotMap &before,
      const SnapshotMap &after)
  {
    std::vector<ChangeInfo> changes;

    const auto diff = fs::snapshot::SnapshotDiff::compute(before, after);

    for (const auto &change : diff)
    {
      fs::events::FileEvent event{
          to_event_type(change.type),
          change.current,
          std::nullopt};

      changes.push_back(ChangeInfo{
          event,
          extract_key_from_event(event)});
    }

    return changes;
  }

  store::types::Key make_store_key(const std::string &value)
  {
    store::types::Key key;
    key.value = value;
    return key;
  }

  store::types::Value make_store_value(const std::vector<std::uint8_t> &payload)
  {
    store::types::Value value;
    value.data = payload;
    return value;
  }

  bool apply_change(
      wal::writer::WalWriter &writer,
      store::engine::StoreEngine &store,
      const ChangeInfo &change)
  {
    wal::core::WalRecord record;
    record.type = to_wal_record_type(change.event.type);
    record.timestamp = static_cast<std::uint64_t>(
        core::time::Timestamp::now().value());
    record.payload = wal::utils::FileEventSerializer::serialize(change.event);

    const auto seq = writer.append(record);

    std::cout << "WAL seq=" << seq
              << " appended for "
              << change.key
              << "\n";

    auto key = make_store_key(change.key);

    switch (change.event.type)
    {
    case fs::types::FileEventType::Created:
    case fs::types::FileEventType::Updated:
    {
      auto value = make_store_value(record.payload);
      const auto result = store.put(key, value);

      std::cout << "Store "
                << (result.success ? "updated" : "update failed")
                << " for key=" << change.key
                << "\n";

      return result.success;
    }

    case fs::types::FileEventType::Deleted:
    {
      const auto result = store.remove(key);

      std::cout << "Store "
                << (result.success ? "removed" : "remove failed")
                << " for key=" << change.key
                << "\n";

      return result.success;
    }
    }

    return false;
  }

  bool path_exists(const std::string &path)
  {
    return std::filesystem::exists(path);
  }

  fs::path::Path require_root_path(const std::string &raw_path)
  {
    auto root_result = fs::path::Path::from(raw_path);

    if (root_result.is_err())
    {
      std::cerr << "Invalid path: " << raw_path << "\n";
      std::exit(1);
    }

    return root_result.value();
  }

  auto build_snapshot_or_exit(const fs::path::Path &root, const std::string &label)
  {
    auto result = fs::snapshot::SnapshotBuilder::build(root);

    if (result.is_err())
    {
      std::cerr << "Error building snapshot (" << label << "): "
                << result.error().message()
                << "\n";
      std::exit(1);
    }

    return result.value();
  }

  wal::writer::WalWriter make_writer(const std::string &wal_path)
  {
    wal::core::WalConfig config;
    config.path = wal_path;
    config.auto_flush = true;

    return wal::writer::WalWriter(config);
  }

  store::engine::StoreEngine make_store(const std::string &wal_path)
  {
    store::core::StoreConfig config;
    config.wal_path = wal_path;
    return store::engine::StoreEngine(config);
  }

  void cleanup_file(const std::string &path)
  {
    if (path_exists(path))
    {
      std::filesystem::remove(path);
    }
  }

  void print_banner()
  {
    std::cout << "== SOFTADASTRA LOCAL RECOVERY DEMO ==\n\n";
  }

  void print_changes(const std::vector<ChangeInfo> &changes)
  {
    if (changes.empty())
    {
      std::cout << "No file changes detected.\n";
      return;
    }

    for (const auto &change : changes)
    {
      std::cout << "Event: "
                << event_type_to_string(change.event.type)
                << " "
                << change.key
                << "\n";
    }
  }

  void wait_for_manual_change()
  {
    std::cout << "[2] Modify files in ./data now...\n";
    std::cout << "Example:\n";
    std::cout << "  echo \"updated\" >> data/note.txt\n";
    std::cout << "Run that in another terminal, then press Enter here.\n\n";
    std::cin.get();
  }

  void run_demo()
  {
    const std::string data_dir = "./data";
    const std::string wal_path = "local_recovery.log";
    const std::string note_path = data_dir + "/note.txt";

    cleanup_file(wal_path);
    std::filesystem::create_directories(data_dir);

    {
      std::ofstream init(note_path, std::ios::trunc);
      init << "hello\n";
    }

    const auto root = require_root_path(data_dir);

    std::vector<std::string> touched_keys;

    print_banner();

    std::cout << "[0] Initial file content:\n";
    {
      std::ifstream in(note_path);
      std::string line;
      while (std::getline(in, line))
      {
        std::cout << "  " << line << "\n";
      }
    }

    std::cout << "\n[1] Building initial snapshot...\n";
    const auto before = build_snapshot_or_exit(root, "before");

    std::cout << "[2] Modifying ./data/note.txt automatically...\n";
    {
      std::ofstream out(note_path, std::ios::app);
      out << "updated by Softadastra demo\n";
    }

    std::cout << "[2.1] File content after modification:\n";
    {
      std::ifstream in(note_path);
      std::string line;
      while (std::getline(in, line))
      {
        std::cout << "  " << line << "\n";
      }
    }

    std::cout << "\n[3] Building updated snapshot...\n";
    const auto after = build_snapshot_or_exit(root, "after");

    std::cout << "[4] Computing diff...\n";
    const auto changes = build_changes(before.all(), after.all());
    print_changes(changes);

    if (changes.empty())
    {
      std::cout << "\nNothing to persist. Exiting.\n";
      return;
    }

    std::cout << "\n[5] Writing to WAL and applying to store...\n";

    {
      auto writer = make_writer(wal_path);
      auto store = make_store(wal_path);

      for (const auto &change : changes)
      {
        touched_keys.push_back(change.key);
        apply_change(writer, store, change);
      }
    }

    std::cout << "\n[6] Simulating restart...\n";

    {
      auto recovered_store = make_store(wal_path);

      std::cout << "[7] Recovering state from WAL-backed store...\n";

      for (const auto &key_str : touched_keys)
      {
        const auto key = make_store_key(key_str);
        const auto entry = recovered_store.get(key);

        if (entry)
        {
          std::cout << "Recovered: "
                    << key_str
                    << " payload_size="
                    << entry->value.data.size()
                    << "\n";
        }
        else
        {
          std::cout << "Recovered: "
                    << key_str
                    << " => deleted/not present\n";
        }
      }
    }

    std::cout << "\nPress Enter to exit...";
    std::cin.get();

    std::cout << "\nDone.\n";
  }
}

int main()
{
  run_demo();
  return 0;
}
