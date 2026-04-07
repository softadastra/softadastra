/*
 * sync_bridge.cpp
 */

#include <iostream>
#include <thread>
#include <chrono>

#include <softadastra/store/core/Operation.hpp>
#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/OperationType.hpp>
#include <softadastra/store/types/Value.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

using namespace softadastra;

store::types::Value make_value(const std::string &s)
{
  store::types::Value v;
  v.data.assign(s.begin(), s.end());
  return v;
}

int main()
{
  // ===== STORE =====
  store::engine::StoreEngine store({.enable_wal = false});

  // ===== SYNC =====
  sync::core::SyncConfig sync_cfg;
  sync_cfg.node_id = "node-a";
  sync_cfg.auto_queue = true;
  sync_cfg.require_ack = true;

  sync::core::SyncContext sync_ctx;
  sync_ctx.store = &store;
  sync_ctx.config = &sync_cfg;

  sync::engine::SyncEngine sync(sync_ctx);

  // ===== TRANSPORT =====
  transport::core::TransportConfig transport_cfg;
  transport_cfg.bind_host = "0.0.0.0";
  transport_cfg.bind_port = 9001;

  transport::core::TransportContext transport_ctx;
  transport_ctx.config = &transport_cfg;
  transport_ctx.sync = &sync;

  transport::backend::TcpTransportBackend backend(transport_cfg);
  transport::engine::TransportEngine engine(transport_ctx, backend);

  if (!engine.start())
  {
    std::cerr << "Transport failed\n";
    return 1;
  }

  // ===== PEER =====
  transport::core::PeerInfo peer;
  peer.node_id = "node-b";
  peer.host = "127.0.0.1";
  peer.port = 9000;

  engine.connect_peer(peer);
  engine.send_hello(peer);

  // ===== LOCAL OPERATION =====
  store::core::Operation op;
  op.type = store::types::OperationType::Put;
  op.key = {"user:1"};
  op.value = make_value("alice");
  op.timestamp = 1000;

  auto sync_op = sync.submit_local_operation(op);

  std::cout << "Local operation created: " << sync_op.sync_id << "\n";

  // ===== SEND LOOP =====
  while (true)
  {
    // send batch
    auto batch = sync.next_batch();

    if (!batch.empty())
    {
      std::size_t sent = engine.send_sync_batch(peer, batch);
      std::cout << "Sent batch: " << sent << "\n";
    }

    // receive
    engine.poll_many(10);

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return 0;
}
