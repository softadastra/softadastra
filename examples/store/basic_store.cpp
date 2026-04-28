/*
 * basic_store.cpp
 */

#include <iostream>

#include <softadastra/store/engine/StoreEngine.hpp>

using namespace softadastra::store;

int main()
{
  core::StoreConfig config;
  config.wal_path = "example.log";

  engine::StoreEngine store(config);

  types::Key key;
  key.value = "username";

  types::Value value;
  value.data = {'g', 'a', 's', 'p', 'a', 'r', 'd'};

  // PUT
  auto res = store.put(key, value);
  std::cout << "PUT success: " << res.success << "\n";

  // GET
  auto entry = store.get(key);
  if (entry)
  {
    std::string name(entry->value.data.begin(), entry->value.data.end());
    std::cout << "GET: " << name << "\n";
  }

  // DELETE
  store.remove(key);

  std::cout << "Example done\n";
}
