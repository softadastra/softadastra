/*
 * wal_usage.cpp
 */

#include <iostream>

#include <softadastra/wal/writer/WalWriter.hpp>
#include <softadastra/wal/reader/WalReader.hpp>

using namespace softadastra::wal;

int main()
{
  core::WalConfig config;
  config.path = "wal_example.log";

  writer::WalWriter writer(config);

  core::WalRecord record;
  record.type = types::WalRecordType::Put;
  record.timestamp = 123;

  record.payload = {1, 2, 3, 4};

  writer.append(record);

  reader::WalReader reader("wal_example.log");

  reader.for_each([](const core::WalRecord &r)
                  { std::cout << "Record seq=" << r.sequence
                              << " payload size=" << r.payload.size() << "\n"; });

  std::cout << "WAL example done\n";
}
