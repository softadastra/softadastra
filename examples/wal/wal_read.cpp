/*
 * wal_read.cpp
 */

#include <iostream>

#include <softadastra/wal/reader/WalReader.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL READ EXAMPLE ==\n";

  const std::string path = "example_wal.log";

  reader::WalReader reader(path);

  auto records = reader.read_all();

  std::cout << "Total records: " << records.size() << "\n\n";

  for (const auto &r : records)
  {
    std::cout << "Sequence:  " << r.sequence << "\n";
    std::cout << "Timestamp: " << r.timestamp << "\n";
    std::cout << "Payload:   ";

    for (auto b : r.payload)
      std::cout << int(b) << " ";

    std::cout << "\n----------------------\n";
  }

  return 0;
}
