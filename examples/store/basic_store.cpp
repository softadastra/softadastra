/*
 * basic_store.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE BASIC EXAMPLE ==\n";

  const std::string wal_path = "basic_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine engine{
      store::core::StoreConfig::durable(wal_path)};

  auto put_result = engine.put(
      store::types::Key{"user:1"},
      store::types::Value::from_string("Gaspard"));

  if (put_result.is_err())
  {
    std::cerr << "put failed: "
              << put_result.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Put version="
            << put_result.value().version
            << "\n";

  auto entry = engine.get(store::types::Key{"user:1"});

  if (!entry.has_value())
  {
    std::cerr << "entry not found\n";
    return 1;
  }

  std::cout << "Entry key="
            << entry->key.str()
            << " value="
            << entry->value.to_string()
            << " version="
            << entry->version
            << "\n";

  auto remove_result = engine.remove(
      store::types::Key{"user:1"});

  if (remove_result.is_err())
  {
    std::cerr << "remove failed: "
              << remove_result.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Removed="
            << remove_result.value().deleted
            << "\n";

  std::filesystem::remove(wal_path);

  return 0;
}
