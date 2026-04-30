/*
 * manual_queue.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== MANUAL QUEUE EXAMPLE ==\n";

  const std::string wal_path = "manual_queue_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto config =
      sync::core::SyncConfig::durable("node-a");

  config.auto_queue = false;

  sync::core::SyncContext context{store, config};
  sync::engine::SyncEngine engine{context};

  auto operation = store::core::Operation::put(
      store::types::Key{"manual:key"},
      store::types::Value::from_string("manual-value"));

  auto submitted = engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "submit failed\n";
    return 1;
  }

  std::cout << "Queued before manual queue: "
            << engine.state().queued_count
            << "\n";

  const auto &sync_id = submitted.value().sync_id;

  if (!engine.queue_operation(sync_id))
  {
    std::cerr << "failed to queue operation\n";
    return 1;
  }

  std::cout << "Queued after manual queue: "
            << engine.state().queued_count
            << "\n";

  auto batch = engine.next_batch();

  std::cout << "Batch size: "
            << batch.size()
            << "\n";

  std::filesystem::remove(wal_path);

  return 0;
}
