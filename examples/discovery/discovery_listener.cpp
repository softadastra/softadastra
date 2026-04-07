#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

#include <softadastra/discovery/discovery.hpp>
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

namespace
{
  store::engine::StoreEngine make_store()
  {
    store::core::StoreConfig config;
    config.enable_wal = false;
    return store::engine::StoreEngine(config);
  }

  sync::core::SyncConfig make_sync_config()
  {
    sync::core::SyncConfig config;
    config.node_id = "node-listener";
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig make_transport_config()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = 9302;
    return config;
  }
}

int main()
{
  using namespace std::chrono_literals;

  try
  {
    auto store = make_store();

    auto sync_config = make_sync_config();
    sync::core::SyncContext sync_context;
    sync_context.store = &store;
    sync_context.config = &sync_config;

    sync::engine::SyncEngine sync_engine(sync_context);

    auto transport_config = make_transport_config();
    transport::core::TransportContext transport_context;
    transport_context.config = &transport_config;
    transport_context.sync = &sync_engine;

    transport::backend::TcpTransportBackend transport_backend(transport_config);
    transport::engine::TransportEngine transport_engine(transport_context, transport_backend);

    if (!transport_engine.start())
    {
      std::cerr << "[listener] failed to start transport\n";
      return 1;
    }

    discovery::DiscoveryOptions options;
    options.node_id = "node-listener";
    options.bind_host = "0.0.0.0";
    options.bind_port = 9400;
    options.broadcast_host = "255.255.255.255";
    options.broadcast_port = 9400;
    options.announce_host = "127.0.0.1";
    options.announce_port = 9302;
    options.announce_interval_ms = 3000;
    options.peer_ttl_ms = 15000;
    options.enable_broadcast = true;

    discovery::DiscoveryService discovery(options, transport_engine);

    discovery.onPeerFound([](const discovery::Peer &peer)
                          { std::cout << "[listener] peer found: "
                                      << peer.node_id
                                      << " at "
                                      << peer.host
                                      << ":"
                                      << peer.port
                                      << '\n'; });

    if (!discovery.start())
    {
      std::cerr << "[listener] failed to start discovery\n";
      transport_engine.stop();
      return 1;
    }

    std::cout << "[listener] started\n";
    std::cout << "[listener] waiting for peers...\n";

    for (int i = 0; i < 60; ++i)
    {
      discovery.poll_many(16);
      transport_engine.poll_many(16);
      std::this_thread::sleep_for(500ms);
    }

    discovery.stop();
    transport_engine.stop();

    std::cout << "[listener] stopped\n";
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[listener] fatal exception: " << ex.what() << '\n';
    return 1;
  }
}
