/*
 * remote_apply.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== REMOTE APPLY EXAMPLE ==\n";

  const std::string wal_path = "remote_apply_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto config =
      sync::core::SyncConfig::durable("node-local");

  sync::core::SyncContext context{store, config};
  sync::engine::SyncEngine engine{context};

  auto remote_store_operation = store::core::Operation::put(
      store::types::Key{"remote:user:1"},
      store::types::Value::from_string("Remote value"));

  auto remote_sync_operation =
      sync::core::SyncOperation::remote(
          "node-remote-1",
          "node-remote",
          1,
          remote_store_operation);

  auto result =
      engine.receive_remote_operation(remote_sync_operation);

  if (result.is_err())
  {
    std::cerr << "remote apply failed: "
              << result.error().message()
              << "\n";
    return 1;
  }

  if (result.value().applied)
  {
    std::cout << "Remote operation applied\n";
  }

  auto entry = store.get(
      store::types::Key{"remote:user:1"});

  if (entry.has_value())
  {
    std::cout << "Stored value: "
              << entry->value.to_string()
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
