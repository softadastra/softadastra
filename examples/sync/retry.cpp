/*
 * retry.cpp
 */

#include <iostream>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/sync/scheduler/SyncScheduler.hpp>

using namespace softadastra;

store::types::Value make_value(const std::string &s)
{
  store::types::Value v;
  v.data.assign(s.begin(), s.end());
  return v;
}

int main()
{
  store::engine::StoreEngine store({.enable_wal = false});

  sync::core::SyncConfig cfg;
  cfg.node_id = "node-a";
  cfg.auto_queue = true;
  cfg.require_ack = true;
  cfg.ack_timeout_ms = 1; // force timeout
  cfg.retry_interval_ms = 1;

  sync::core::SyncContext ctx;
  ctx.store = &store;
  ctx.config = &cfg;

  sync::engine::SyncEngine engine(ctx);
  sync::scheduler::SyncScheduler scheduler(engine);

  // Submit
  store::core::Operation op;
  op.type = store::types::OperationType::Put;
  op.key = {"k1"};
  op.value = make_value("v1");
  op.timestamp = 1000;

  engine.submit_local_operation(op);

  auto batch = engine.next_batch();

  std::cout << "Sent batch: " << batch.size() << "\n";

  // Simulate timeout
  auto retried = scheduler.retry_only();

  std::cout << "Retried: " << retried << "\n";

  return 0;
}
