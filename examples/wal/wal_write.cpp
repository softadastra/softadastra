/*
 * wal_write.cpp
 */

#include <iostream>
#include <filesystem>

#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/wal/core/WalRecord.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL WRITE EXAMPLE ==\n";

  const std::string path = "example_wal.log";
  std::filesystem::remove(path);

  core::WalConfig config;
  config.path = path;
  config.auto_flush = true;

  writer::WalWriter writer(config);

  // Record 1
  core::WalRecord r1;
  r1.type = types::WalRecordType::Put;
  r1.timestamp = 1000;
  r1.payload = {1, 2, 3};

  auto seq1 = writer.append(r1);

  std::cout << "Written record seq=" << seq1 << "\n";

  // Record 2
  core::WalRecord r2;
  r2.type = types::WalRecordType::Update;
  r2.timestamp = 2000;
  r2.payload = {4, 5};

  auto seq2 = writer.append(r2);

  std::cout << "Written record seq=" << seq2 << "\n";

  writer.flush();

  std::cout << "WAL written to: " << path << "\n";

  return 0;
}
