#include <chrono>
#include <cstdint>
#include <exception>
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

  sync::core::SyncConfig make_sync_config(const std::string &node_id)
  {
    sync::core::SyncConfig config;
    config.node_id = node_id;
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig make_transport_config(std::uint16_t port)
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = port;
    return config;
  }
}

int main()
{
  using namespace std::chrono_literals;

  try
  {
    const std::string node_id = "node-minimal";
    const std::uint16_t transport_port = 9301;
    const std::uint16_t discovery_port = 9400;

    auto store = make_store();

    auto sync_config = make_sync_config(node_id);
    sync::core::SyncContext sync_context;
    sync_context.store = &store;
    sync_context.config = &sync_config;

    sync::engine::SyncEngine sync_engine(sync_context);

    auto transport_config = make_transport_config(transport_port);
    transport::core::TransportContext transport_context;
    transport_context.config = &transport_config;
    transport_context.sync = &sync_engine;

    transport::backend::TcpTransportBackend transport_backend(transport_config);
    transport::engine::TransportEngine transport_engine(transport_context, transport_backend);

    if (!transport_engine.start())
    {
      std::cerr << "[minimal] failed to start transport\n";
      return 1;
    }

    discovery::DiscoveryOptions options;
    options.node_id = node_id;
    options.bind_host = "0.0.0.0";
    options.bind_port = discovery_port;
    options.broadcast_host = "255.255.255.255";
    options.broadcast_port = discovery_port;
    options.announce_host = "127.0.0.1";
    options.announce_port = transport_port;
    options.announce_interval_ms = 3000;
    options.peer_ttl_ms = 15000;
    options.enable_broadcast = true;

    discovery::DiscoveryService discovery(options, transport_engine);

    discovery.onPeerFound([](const discovery::Peer &peer)
                          { std::cout << "[minimal] peer found: "
                                      << peer.node_id
                                      << " at "
                                      << peer.host
                                      << ":"
                                      << peer.port
                                      << '\n'; });

    if (!discovery.start())
    {
      std::cerr << "[minimal] failed to start discovery\n";
      transport_engine.stop();
      return 1;
    }

    std::cout << "[minimal] discovery started\n";
    std::cout << "[minimal] node_id=" << node_id << '\n';
    std::cout << "[minimal] transport_port=" << transport_port << '\n';
    std::cout << "[minimal] discovery_port=" << discovery_port << '\n';
    std::cout << "[minimal] waiting for peers...\n";

    discovery.probe_now();

    for (int i = 0; i < 60; ++i)
    {
      discovery.poll_many(16);
      transport_engine.poll_many(16);
      std::this_thread::sleep_for(500ms);
    }

    const auto peers = discovery.peers();

    std::cout << "[minimal] discovered peers: " << peers.size() << '\n';
    for (const auto &peer : peers)
    {
      std::cout << "  - "
                << peer.node_id
                << " at "
                << peer.host
                << ":"
                << peer.port
                << '\n';
    }

    discovery.stop();
    transport_engine.stop();

    std::cout << "[minimal] stopped\n";
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[minimal] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[minimal] fatal unknown exception\n";
    return 1;
  }
}
