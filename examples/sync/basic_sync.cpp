/*
 * basic_sync.cpp
 */

#include <iostream>

#include <softadastra/store/core/Operation.hpp>
#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/OperationType.hpp>
#include <softadastra/store/types/Value.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>

using namespace softadastra;

store::types::Value make_value(const std::string &s)
{
  store::types::Value v;
  v.data.assign(s.begin(), s.end());
  return v;
}

int main()
{
  // 1. Store
  store::core::StoreConfig store_cfg;
  store_cfg.enable_wal = false;

  store::engine::StoreEngine store(store_cfg);

  // 2. Sync config
  sync::core::SyncConfig sync_cfg;
  sync_cfg.node_id = "node-a";
  sync_cfg.auto_queue = true;
  sync_cfg.require_ack = true;

  sync::core::SyncContext ctx;
  ctx.store = &store;
  ctx.config = &sync_cfg;

  sync::engine::SyncEngine engine(ctx);

  // 3. Create operation
  store::core::Operation op;
  op.type = store::types::OperationType::Put;
  op.key = {"user:1"};
  op.value = make_value("alice");
  op.timestamp = 1000;

  // 4. Submit
  auto sync_op = engine.submit_local_operation(op);

  std::cout << "Submitted sync_id: " << sync_op.sync_id << "\n";

  // 5. Get batch
  auto batch = engine.next_batch();

  std::cout << "Batch size: " << batch.size() << "\n";

  // 6. Simulate ack
  engine.receive_ack(batch[0].operation.sync_id);

  std::cout << "Ack received\n";

  return 0;
}
