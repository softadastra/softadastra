/*
 * retry.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== SYNC RETRY EXAMPLE ==\n";

  const std::string wal_path = "retry_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto config = sync::core::SyncConfig::fast("node-a");
  config.require_ack = true;
  config.max_retries = 3;
  config.ack_timeout = core::time::Duration::from_millis(1);
  config.retry_interval = core::time::Duration::from_millis(1);

  sync::core::SyncContext context{store, config};
  sync::engine::SyncEngine engine{context};

  auto operation = store::core::Operation::put(
      store::types::Key{"retry:key"},
      store::types::Value::from_string("retry-value"));

  auto submitted = engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "submit failed\n";
    return 1;
  }

  auto batch = engine.next_batch();

  std::cout << "Initial batch size: "
            << batch.size()
            << "\n";

  auto retried = engine.retry_expired();

  std::cout << "Retried count: "
            << retried
            << "\n";

  std::cout << "Outbox size: "
            << engine.state().outbox_size
            << "\n";

  std::filesystem::remove(wal_path);

  return 0;
}
