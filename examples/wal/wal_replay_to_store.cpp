/*
 * wal_replay_to_store.cpp
 */

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/reader/WalReader.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra::wal;

/*
 * Small demo store:
 * key -> raw bytes
 */
class DemoStore
{
public:
  void apply(const core::WalRecord &record)
  {
    const std::string key = "record_" + std::to_string(record.sequence);

    switch (record.type)
    {
    case types::WalRecordType::Put:
    case types::WalRecordType::Update:
      data_[key] = record.payload;
      break;

    case types::WalRecordType::Delete:
      data_.erase(key);
      break;

    default:
      break;
    }
  }

  void print() const
  {
    std::cout << "\n== STORE STATE ==\n";

    if (data_.empty())
    {
      std::cout << "(empty)\n";
      return;
    }

    for (const auto &[key, value] : data_)
    {
      std::cout << key << " => [ ";
      for (auto byte : value)
      {
        std::cout << static_cast<int>(byte) << ' ';
      }
      std::cout << "]\n";
    }
  }

private:
  std::map<std::string, std::vector<std::uint8_t>> data_;
};

int main()
{
  std::cout << "== WAL REPLAY TO STORE EXAMPLE ==\n";

  const std::string wal_path = "replay_store_example.log";
  std::filesystem::remove(wal_path);

  // 1. Write some records
  core::WalConfig config;
  config.path = wal_path;
  config.auto_flush = true;

  writer::WalWriter writer(config);

  core::WalRecord r1;
  r1.type = types::WalRecordType::Put;
  r1.timestamp = 1001;
  r1.payload = {10, 20, 30};
  writer.append(r1);

  core::WalRecord r2;
  r2.type = types::WalRecordType::Update;
  r2.timestamp = 1002;
  r2.payload = {99, 88};
  writer.append(r2);

  core::WalRecord r3;
  r3.type = types::WalRecordType::Put;
  r3.timestamp = 1003;
  r3.payload = {7, 8, 9, 10};
  writer.append(r3);

  writer.flush();

  std::cout << "WAL written to: " << wal_path << "\n";

  // 2. Replay into a demo store
  reader::WalReader reader(wal_path);
  DemoStore store;

  const auto records = reader.read_all();

  std::cout << "\n== REPLAYING RECORDS ==\n";
  for (const auto &record : records)
  {
    std::cout << "Applying seq=" << record.sequence
              << " type=" << static_cast<int>(record.type)
              << " payload_size=" << record.payload.size()
              << "\n";

    store.apply(record);
  }

  // 3. Final store state
  store.print();

  std::filesystem::remove(wal_path);
  return 0;
}
