/*
 * wal_stream.cpp
 */

#include <iostream>

#include <softadastra/wal/reader/WalReader.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL STREAM EXAMPLE ==\n";

  const std::string path = "example_wal.log";

  reader::WalReader reader(path);

  reader.for_each([](const core::WalRecord &r)
                  { std::cout << "[record] seq=" << r.sequence
                              << " timestamp=" << r.timestamp
                              << " payload_size=" << r.payload.size()
                              << "\n"; });

  return 0;
}
