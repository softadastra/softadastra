/*
 * basic_sync.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== BASIC SYNC EXAMPLE ==\n";

  const std::string wal_path = "basic_sync_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto config =
      sync::core::SyncConfig::durable("node-a");

  sync::core::SyncContext context{store, config};

  if (!context.is_valid())
  {
    std::cerr << "invalid sync context\n";
    return 1;
  }

  sync::engine::SyncEngine engine{context};

  auto operation = store::core::Operation::put(
      store::types::Key{"user:1"},
      store::types::Value::from_string("Gaspard"));

  auto submitted = engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "submit failed: "
              << submitted.error().message()
              << "\n";
    return 1;
  }

  std::cout << "Submitted sync id: "
            << submitted.value().sync_id
            << "\n";

  auto batch = engine.next_batch();

  std::cout << "Batch size: "
            << batch.size()
            << "\n";

  for (const auto &envelope : batch)
  {
    std::cout << "Ready to send: "
              << envelope.operation.sync_id
              << " version="
              << envelope.operation.version
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
