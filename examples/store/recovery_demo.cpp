/*
 * recovery_demo.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE RECOVERY DEMO ==\n";

  const std::string wal_path = "store_recovery_demo.wal";
  std::filesystem::remove(wal_path);

  {
    store::engine::StoreEngine engine{
        store::core::StoreConfig::durable(wal_path)};

    auto a = engine.put(
        store::types::Key{"user:1"},
        store::types::Value::from_string("Gaspard"));

    auto b = engine.put(
        store::types::Key{"user:2"},
        store::types::Value::from_string("Softadastra"));

    auto c = engine.put(
        store::types::Key{"user:1"},
        store::types::Value::from_string("Gaspard Kirira"));

    if (a.is_err() || b.is_err() || c.is_err())
    {
      std::cerr << "initial writes failed\n";
      return 1;
    }

    std::cout << "Before restart size: "
              << engine.size()
              << "\n";
  }

  {
    store::engine::StoreEngine recovered{
        store::core::StoreConfig::durable(wal_path)};

    std::cout << "After recovery size: "
              << recovered.size()
              << "\n";

    auto entry = recovered.get(
        store::types::Key{"user:1"});

    if (!entry.has_value())
    {
      std::cerr << "recovered entry missing\n";
      return 1;
    }

    std::cout << "Recovered user:1 => "
              << entry->value.to_string()
              << " version="
              << entry->version
              << "\n";

    std::cout << "Last recovered sequence: "
              << recovered.last_recovered_sequence()
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
