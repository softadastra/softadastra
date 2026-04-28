#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

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
  struct DemoNode
  {
    std::string node_id;
    std::uint16_t transport_port;
    std::uint16_t discovery_port;

    store::engine::StoreEngine store;
    sync::core::SyncConfig sync_config;
    sync::core::SyncContext sync_context;
    sync::engine::SyncEngine sync;

    transport::core::TransportConfig transport_config;
    transport::core::TransportContext transport_context;
    transport::backend::TcpTransportBackend transport_backend;
    transport::engine::TransportEngine transport_engine;

    discovery::DiscoveryOptions discovery_options;
    discovery::DiscoveryService discovery;

    DemoNode(const std::string &id,
             std::uint16_t tport,
             std::uint16_t dport)
        : node_id(id),
          transport_port(tport),
          discovery_port(dport),
          store(make_store_config()),
          sync_config(make_sync_config(id)),
          sync_context(make_sync_context(store, sync_config)),
          sync(sync_context),
          transport_config(make_transport_config(tport)),
          transport_context(make_transport_context(sync, transport_config)),
          transport_backend(transport_config),
          transport_engine(transport_context, transport_backend),
          discovery_options(make_discovery_options(id, tport, dport)),
          discovery(discovery_options, transport_engine)
    {
    }

    static store::core::StoreConfig make_store_config()
    {
      store::core::StoreConfig config;
      config.enable_wal = false;
      return config;
    }

    static sync::core::SyncConfig make_sync_config(const std::string &node_id)
    {
      sync::core::SyncConfig config;
      config.node_id = node_id;
      config.auto_queue = true;
      config.require_ack = true;
      return config;
    }

    static sync::core::SyncContext make_sync_context(store::engine::StoreEngine &store,
                                                     sync::core::SyncConfig &config)
    {
      sync::core::SyncContext context;
      context.store = &store;
      context.config = &config;
      return context;
    }

    static transport::core::TransportConfig make_transport_config(std::uint16_t port)
    {
      transport::core::TransportConfig config;
      config.bind_host = "0.0.0.0";
      config.bind_port = port;
      return config;
    }

    static transport::core::TransportContext make_transport_context(
        sync::engine::SyncEngine &sync,
        transport::core::TransportConfig &config)
    {
      transport::core::TransportContext context;
      context.config = &config;
      context.sync = &sync;
      return context;
    }

    static discovery::DiscoveryOptions make_discovery_options(
        const std::string &node_id,
        std::uint16_t transport_port,
        std::uint16_t discovery_port)
    {
      discovery::DiscoveryOptions options;
      options.node_id = node_id;
      options.bind_host = "0.0.0.0";
      options.bind_port = discovery_port;
      options.broadcast_host = "255.255.255.255";
      options.broadcast_port = discovery_port;
      options.announce_host = "127.0.0.1";
      options.announce_port = transport_port;
      options.announce_interval_ms = 2000;
      options.peer_ttl_ms = 10000;
      options.enable_broadcast = true;
      return options;
    }

    bool start()
    {
      if (!transport_engine.start())
      {
        return false;
      }

      if (!discovery.start())
      {
        transport_engine.stop();
        return false;
      }

      return true;
    }

    void stop()
    {
      discovery.stop();
      transport_engine.stop();
    }

    void tick()
    {
      discovery.poll_many(16);
      transport_engine.poll_many(16);
    }

    bool announce()
    {
      return discovery.announce_now();
    }

    bool probe()
    {
      return discovery.probe_now();
    }

    bool has_peer(const std::string &peer_id) const
    {
      for (const auto &peer : discovery.peers())
      {
        if (peer.node_id == peer_id)
        {
          return true;
        }
      }

      return false;
    }

    void print_peers() const
    {
      const auto peers = discovery.peers();

      std::cout << "[" << node_id << "] discovered peers:\n";

      if (peers.empty())
      {
        std::cout << "  - none\n";
        return;
      }

      for (const auto &peer : peers)
      {
        std::cout
            << "  - node_id=" << peer.node_id
            << ", host=" << peer.host
            << ", port=" << peer.port
            << ", last_seen_at=" << peer.last_seen_at
            << '\n';
      }
    }
  };
}

int main()
{
  try
  {
    using namespace std::chrono_literals;

    DemoNode node_a("node-a", 9301, 9400);
    DemoNode node_b("node-b", 9302, 9400);

    node_a.discovery.onPeerFound([](const discovery::Peer &peer)
                                 { std::cout << "[node-a] peer found: " << peer.node_id
                                             << " at " << peer.host << ":" << peer.port << '\n'; });

    node_b.discovery.onPeerFound([](const discovery::Peer &peer)
                                 { std::cout << "[node-b] peer found: " << peer.node_id
                                             << " at " << peer.host << ":" << peer.port << '\n'; });

    if (!node_a.start())
    {
      std::cerr << "[demo] failed to start node-a\n";
      return 1;
    }

    if (!node_b.start())
    {
      std::cerr << "[demo] failed to start node-b\n";
      node_a.stop();
      return 1;
    }

    std::cout << "[demo] both nodes started\n";

    node_a.probe();
    node_b.probe();

    bool converged = false;

    for (int i = 0; i < 50; ++i)
    {
      node_a.announce();
      node_b.announce();

      node_a.tick();
      node_b.tick();

      if (node_a.has_peer("node-b") && node_b.has_peer("node-a"))
      {
        converged = true;
        break;
      }

      std::this_thread::sleep_for(200ms);
    }

    node_a.print_peers();
    node_b.print_peers();

    node_a.stop();
    node_b.stop();

    if (!converged)
    {
      std::cerr << "[demo] discovery roundtrip did not converge\n";
      return 1;
    }

    std::cout << "[demo] discovery roundtrip completed successfully\n";
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[demo] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[demo] fatal unknown exception\n";
    return 1;
  }
}
