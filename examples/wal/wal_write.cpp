/*
 * wal_write.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/wal/Wal.hpp>

using namespace softadastra::wal;

int main()
{
  std::cout << "== WAL WRITE EXAMPLE ==\n";

  const std::string path = "example_wal.log";
  std::filesystem::remove(path);

  writer::WalWriter writer{
      core::WalConfig::durable(path)};

  auto first = writer.append(
      types::WalRecordType::Put,
      core::WalRecord::Payload{1, 2, 3});

  if (first.is_err())
  {
    std::cerr << "Failed to write first record: "
              << first.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Written record seq=" << first.value() << "\n";

  auto second = writer.append(
      types::WalRecordType::Update,
      core::WalRecord::Payload{4, 5});

  if (second.is_err())
  {
    std::cerr << "Failed to write second record: "
              << second.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Written record seq=" << second.value() << "\n";

  auto flushed = writer.flush();

  if (flushed.is_err())
  {
    std::cerr << "Failed to flush WAL: "
              << flushed.error().message()
              << "\n";
    return 1;
  }

  std::cout << "WAL written to: " << path << "\n";

  return 0;
}
