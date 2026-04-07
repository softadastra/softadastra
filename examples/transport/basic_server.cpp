/*
 * basic_server.cpp
 */

#include <iostream>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

using namespace softadastra;

int main()
{
  // Store
  store::engine::StoreEngine store({.enable_wal = false});

  // Sync
  sync::core::SyncConfig sync_cfg;
  sync_cfg.node_id = "node-server";

  sync::core::SyncContext sync_ctx;
  sync_ctx.store = &store;
  sync_ctx.config = &sync_cfg;

  sync::engine::SyncEngine sync(sync_ctx);

  // Transport
  transport::core::TransportConfig transport_cfg;
  transport_cfg.bind_host = "0.0.0.0";
  transport_cfg.bind_port = 9000;

  transport::core::TransportContext transport_ctx;
  transport_ctx.config = &transport_cfg;
  transport_ctx.sync = &sync;

  transport::backend::TcpTransportBackend backend(transport_cfg);
  transport::engine::TransportEngine engine(transport_ctx, backend);

  if (!engine.start())
  {
    std::cerr << "Failed to start server\n";
    return 1;
  }

  std::cout << "Server listening on port 9000...\n";

  while (true)
  {
    engine.poll_many(10);
  }

  return 0;
}
