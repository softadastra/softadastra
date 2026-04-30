/*
 * scheduler_tick.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== SYNC SCHEDULER TICK EXAMPLE ==\n";

  const std::string wal_path = "scheduler_tick_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto config =
      sync::core::SyncConfig::durable("node-a");

  sync::core::SyncContext context{store, config};
  sync::engine::SyncEngine engine{context};
  sync::scheduler::SyncScheduler scheduler{engine};

  auto operation = store::core::Operation::put(
      store::types::Key{"task:1"},
      store::types::Value::from_string("sync-me"));

  auto submitted = engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "submit failed\n";
    return 1;
  }

  auto tick = scheduler.tick(false);

  std::cout << "Tick batch size: "
            << tick.batch_size()
            << "\n";

  std::cout << "Tick has work: "
            << tick.has_work()
            << "\n";

  for (const auto &envelope : tick.batch)
  {
    std::cout << "Transport should send: "
              << envelope.operation.sync_id
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
