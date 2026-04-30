/*
 * wal_usage.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE WAL USAGE EXAMPLE ==\n";

  const std::string wal_path = "store_wal_usage.wal";
  std::filesystem::remove(wal_path);

  auto config = store::core::StoreConfig::durable(wal_path);

  store::engine::StoreEngine engine{config};

  auto first = engine.put(
      store::types::Key{"product:1"},
      store::types::Value::from_string("Laptop"));

  auto second = engine.put(
      store::types::Key{"product:2"},
      store::types::Value::from_string("Phone"));

  auto third = engine.put(
      store::types::Key{"product:1"},
      store::types::Value::from_string("Laptop Pro"));

  if (first.is_err() || second.is_err() || third.is_err())
  {
    std::cerr << "failed to write one or more entries\n";
    return 1;
  }

  std::cout << "Current store size: "
            << engine.size()
            << "\n";

  for (const auto &[key, entry] : engine.entries())
  {
    std::cout << key
              << " => "
              << entry.value.to_string()
              << " version="
              << entry.version
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
