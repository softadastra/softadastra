#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <thread>

#include <softadastra/discovery/discovery.hpp>
#include <softadastra/metadata/metadata.hpp>
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
    const std::string node_id = "node-metadata";
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
      std::cerr << "[metadata] failed to start transport\n";
      return 1;
    }

    discovery::DiscoveryOptions discovery_options;
    discovery_options.node_id = node_id;
    discovery_options.bind_host = "0.0.0.0";
    discovery_options.bind_port = discovery_port;
    discovery_options.broadcast_host = "255.255.255.255";
    discovery_options.broadcast_port = discovery_port;
    discovery_options.announce_host = "127.0.0.1";
    discovery_options.announce_port = transport_port;
    discovery_options.announce_interval_ms = 3000;
    discovery_options.peer_ttl_ms = 15000;
    discovery_options.enable_broadcast = true;

    discovery::DiscoveryService discovery(discovery_options, transport_engine);

    if (!discovery.start())
    {
      std::cerr << "[metadata] failed to start discovery\n";
      transport_engine.stop();
      return 1;
    }

    metadata::MetadataOptions metadata_options;
    metadata_options.node_id = node_id;
    metadata_options.display_name = "Metadata Demo Node";
    metadata_options.version = "0.1.0";
    metadata_options.auto_refresh = true;
    metadata_options.refresh_interval_ms = 5000;

    metadata::MetadataService metadata(metadata_options, discovery.engine());

    if (!metadata.start())
    {
      std::cerr << "[metadata] failed to start metadata\n";
      discovery.stop();
      transport_engine.stop();
      return 1;
    }

    auto local = metadata.local_or_refresh();
    if (!local.has_value())
    {
      std::cerr << "[metadata] no local metadata available\n";
      metadata.stop();
      discovery.stop();
      transport_engine.stop();
      return 1;
    }

    std::cout << "[metadata] local metadata loaded\n";
    std::cout << "node_id:      " << local->identity.node_id << '\n';
    std::cout << "display_name: " << local->identity.display_name << '\n';
    std::cout << "hostname:     " << local->runtime.hostname << '\n';
    std::cout << "os_name:      " << local->runtime.os_name << '\n';
    std::cout << "version:      " << local->runtime.version << '\n';
    std::cout << "started_at:   " << local->runtime.started_at << '\n';
    std::cout << "uptime_ms:    " << local->runtime.uptime_ms << '\n';
    std::cout << "capabilities: " << local->capabilities.values.size() << '\n';

    std::this_thread::sleep_for(1s);

    auto refreshed = metadata.refresh();
    if (refreshed.has_value())
    {
      std::cout << "[metadata] refreshed uptime_ms=" << refreshed->runtime.uptime_ms << '\n';
    }

    metadata.stop();
    discovery.stop();
    transport_engine.stop();

    std::cout << "[metadata] stopped\n";
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[metadata] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[metadata] fatal unknown exception\n";
    return 1;
  }
}
