/*
 * remote_apply.cpp
 */

#include <iostream>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/core/SyncOperation.hpp>
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
  // Node A (receiver)
  store::engine::StoreEngine store({.enable_wal = false});

  sync::core::SyncConfig cfg;
  cfg.node_id = "node-a";

  sync::core::SyncContext ctx;
  ctx.store = &store;
  ctx.config = &cfg;

  sync::engine::SyncEngine engine(ctx);

  // Simulate remote op from node-b
  sync::core::SyncOperation remote;

  remote.sync_id = "node-b-1";
  remote.origin_node_id = "node-b";
  remote.direction = sync::types::SyncDirection::RemoteToLocal;

  remote.op.type = store::types::OperationType::Put;
  remote.op.key = {"user:42"};
  remote.op.value = make_value("bob");
  remote.op.timestamp = 2000;

  auto result = engine.receive_remote_operation(remote);

  std::cout << "Applied: " << result.applied << "\n";

  auto entry = store.get({"user:42"});

  if (entry)
  {
    std::cout << "Value size: " << entry->value.size() << "\n";
  }

  return 0;
}
