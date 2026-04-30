/*
 * wal_replay_to_store.cpp
 */

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <softadastra/wal/Wal.hpp>

using namespace softadastra::wal;

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
  std::map<std::string, std::vector<std::uint8_t>> data_{};
};

int main()
{
  std::cout << "== WAL REPLAY TO STORE EXAMPLE ==\n";

  const std::string wal_path = "replay_store_example.log";
  std::filesystem::remove(wal_path);

  writer::WalWriter writer{
      core::WalConfig::durable(wal_path)};

  auto r1 = writer.append(
      types::WalRecordType::Put,
      core::WalRecord::Payload{10, 20, 30});

  auto r2 = writer.append(
      types::WalRecordType::Update,
      core::WalRecord::Payload{99, 88});

  auto r3 = writer.append(
      types::WalRecordType::Put,
      core::WalRecord::Payload{7, 8, 9, 10});

  if (r1.is_err() || r2.is_err() || r3.is_err())
  {
    std::cerr << "Failed to write WAL records\n";
    return 1;
  }

  auto flushed = writer.flush();

  if (flushed.is_err())
  {
    std::cerr << "Failed to flush WAL: "
              << flushed.error().message()
              << "\n";
    return 1;
  }

  std::cout << "WAL written to: " << wal_path << "\n";

  DemoStore store;
  replay::WalReplayer replayer{wal_path};

  std::cout << "\n== REPLAYING RECORDS ==\n";

  auto replay_result = replayer.replay(
      [&](const core::WalRecord &record)
      {
        std::cout << "Applying seq=" << record.sequence
                  << " type=" << types::to_string(record.type)
                  << " payload_size=" << record.payload.size()
                  << "\n";

        store.apply(record);
      });

  if (replay_result.is_err())
  {
    std::cerr << "Replay failed: "
              << replay_result.error().message()
              << "\n";
    return 1;
  }

  store.print();

  std::filesystem::remove(wal_path);

  return 0;
}
