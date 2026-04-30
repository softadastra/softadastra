/*
 * memory_only.cpp
 */

#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE MEMORY ONLY EXAMPLE ==\n";

  store::engine::StoreEngine engine{
      store::core::StoreConfig::memory_only()};

  auto result = engine.put(
      store::types::Key{"session:1"},
      store::types::Value::from_string("temporary-value"));

  if (result.is_err())
  {
    std::cerr << "put failed: "
              << result.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Created="
            << result.value().created
            << " version="
            << result.value().version
            << "\n";

  auto entry = engine.get(
      store::types::Key{"session:1"});

  if (entry.has_value())
  {
    std::cout << "session:1 => "
              << entry->value.to_string()
              << "\n";
  }

  return 0;
}
