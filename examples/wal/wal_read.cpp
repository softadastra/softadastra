/*
 * wal_read.cpp
 */

#include <iostream>

#include <softadastra/wal/Wal.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL READ EXAMPLE ==\n";

  const std::string path = "example_wal.log";

  reader::WalReader reader{path};

  auto records = reader.read_all();

  if (records.is_err())
  {
    std::cerr << "Failed to read WAL: "
              << records.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Total records: " << records.value().size() << "\n\n";

  for (const auto &record : records.value())
  {
    std::cout << "Sequence:  " << record.sequence << "\n";
    std::cout << "Type:      " << types::to_string(record.type) << "\n";
    std::cout << "Status:    " << types::to_string(record.status) << "\n";
    std::cout << "Timestamp: " << record.timestamp.millis() << "\n";
    std::cout << "Payload:   ";

    for (auto byte : record.payload)
    {
      std::cout << static_cast<int>(byte) << ' ';
    }

    std::cout << "\n----------------------\n";
  }

  return 0;
}
