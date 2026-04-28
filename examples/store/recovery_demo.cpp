/*
 * recovery_demo.cpp
 */

#include <iostream>

#include <softadastra/store/engine/StoreEngine.hpp>

using namespace softadastra::store;

int main()
{
  std::cout << "=== First run ===\n";

  {
    core::StoreConfig config;
    config.wal_path = "recovery.log";

    engine::StoreEngine store(config);

    types::Key key;
    key.value = "session";

    types::Value value;
    value.data = {42};

    store.put(key, value);

    std::cout << "Data written\n";
  }

  std::cout << "\n=== Simulating restart ===\n";

  {
    core::StoreConfig config;
    config.wal_path = "recovery.log";

    engine::StoreEngine store(config);

    types::Key key;
    key.value = "session";

    auto entry = store.get(key);

    if (entry)
    {
      std::cout << "Recovered value: "
                << (int)entry->value.data[0] << "\n";
    }
    else
    {
      std::cout << "Recovery failed\n";
    }
  }

  std::cout << "Recovery demo done\n";
}
