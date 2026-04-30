/*
 * wal_stream.cpp
 */

#include <iostream>

#include <softadastra/wal/Wal.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL STREAM EXAMPLE ==\n";

  const std::string path = "example_wal.log";

  reader::WalReader reader{path};

  auto result = reader.for_each(
      [](const core::WalRecord &record)
      {
        std::cout << "[record] seq=" << record.sequence
                  << " type=" << types::to_string(record.type)
                  << " status=" << types::to_string(record.status)
                  << " timestamp=" << record.timestamp.millis()
                  << " payload_size=" << record.payload.size()
                  << "\n";
      });

  if (result.is_err())
  {
    std::cerr << "Failed to stream WAL: "
              << result.error().message()
              << "\n";
    return 1;
  }

  return 0;
}
