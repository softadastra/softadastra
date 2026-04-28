/*
 * metadata_registry_demo.cpp
 *
 * Metadata registry demo:
 * - starts metadata engine
 * - inserts local metadata
 * - inserts one synthetic remote metadata entry
 * - prints registry contents
 */

#include <exception>
#include <iostream>
#include <optional>
#include <string>

#include <softadastra/discovery/backend/UdpDiscoveryBackend.hpp>
#include <softadastra/discovery/core/DiscoveryConfig.hpp>
#include <softadastra/discovery/core/DiscoveryContext.hpp>
#include <softadastra/discovery/engine/DiscoveryEngine.hpp>

#include <softadastra/metadata/core/MetadataConfig.hpp>
#include <softadastra/metadata/core/MetadataContext.hpp>
#include <softadastra/metadata/core/NodeCapabilities.hpp>
#include <softadastra/metadata/core/NodeIdentity.hpp>
#include <softadastra/metadata/core/NodeMetadata.hpp>
#include <softadastra/metadata/core/NodeRuntimeInfo.hpp>
#include <softadastra/metadata/engine/MetadataEngine.hpp>
#include <softadastra/metadata/types/CapabilityType.hpp>

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
  constexpr const char *kNodeId = "metadata-registry";
  constexpr std::uint16_t kTransportPort = 9502;
  constexpr std::uint16_t kDiscoveryPort = 9601;

  void log_info(const std::string &message)
  {
    std::cout << "[metadata-registry] " << message << '\n';
  }

  store::core::StoreConfig build_store_config()
  {
    store::core::StoreConfig config;
    config.enable_wal = false;
    return config;
  }

  sync::core::SyncConfig build_sync_config()
  {
    sync::core::SyncConfig config;
    config.node_id = kNodeId;
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig build_transport_config()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = kTransportPort;
    return config;
  }

  discovery::core::DiscoveryConfig build_discovery_config()
  {
    discovery::core::DiscoveryConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = kDiscoveryPort;
    config.broadcast_host = "255.255.255.255";
    config.broadcast_port = kDiscoveryPort;
    config.node_id = kNodeId;
    config.announce_host = "127.0.0.1";
    config.announce_port = kTransportPort;
    config.announce_interval_ms = 3000;
    config.peer_ttl_ms = 15000;
    config.max_datagram_size = 64 * 1024;
    config.enable_broadcast = true;
    return config;
  }

  metadata::core::MetadataConfig build_metadata_config()
  {
    metadata::core::MetadataConfig config;
    config.node_id = kNodeId;
    config.display_name = "Metadata Registry Demo";
    config.version = "0.1.0";
    config.auto_refresh = true;
    config.refresh_interval_ms = 5000;
    return config;
  }

  std::string capability_to_string(metadata::types::CapabilityType capability)
  {
    using CapabilityType = metadata::types::CapabilityType;

    switch (capability)
    {
    case CapabilityType::Core:
      return "Core";
    case CapabilityType::Fs:
      return "Fs";
    case CapabilityType::Wal:
      return "Wal";
    case CapabilityType::Store:
      return "Store";
    case CapabilityType::Sync:
      return "Sync";
    case CapabilityType::Transport:
      return "Transport";
    case CapabilityType::Discovery:
      return "Discovery";
    case CapabilityType::Metadata:
      return "Metadata";
    case CapabilityType::App:
      return "App";
    case CapabilityType::Cli:
      return "Cli";
    case CapabilityType::Unknown:
    default:
      return "Unknown";
    }
  }

  void print_registry(const metadata::engine::MetadataEngine &engine)
  {
    const auto all = engine.registry().all();

    std::cout << "registry:\n";

    if (all.empty())
    {
      std::cout << "  - empty\n";
      return;
    }

    for (const auto &meta : all)
    {
      std::cout << "  - node_id=" << meta.identity.node_id
                << ", display_name=" << meta.identity.display_name
                << ", hostname=" << meta.runtime.hostname
                << ", os_name=" << meta.runtime.os_name
                << ", version=" << meta.runtime.version
                << ", capabilities=";

      if (meta.capabilities.values.empty())
      {
        std::cout << "none";
      }
      else
      {
        bool first = true;
        for (const auto capability : meta.capabilities.values)
        {
          if (!first)
          {
            std::cout << ", ";
          }
          first = false;
          std::cout << capability_to_string(capability);
        }
      }

      std::cout << '\n';
    }
  }

  metadata::core::NodeMetadata make_remote_metadata()
  {
    metadata::core::NodeMetadata meta;

    meta.identity.node_id = "remote-node-1";
    meta.identity.display_name = "Synthetic Remote Node";

    meta.runtime.hostname = "remote-host";
    meta.runtime.os_name = "linux";
    meta.runtime.version = "0.1.0";
    meta.runtime.started_at = 1000;
    meta.runtime.uptime_ms = 42000;

    meta.capabilities.values = {
        metadata::types::CapabilityType::Core,
        metadata::types::CapabilityType::Store,
        metadata::types::CapabilityType::Sync,
        metadata::types::CapabilityType::Transport,
        metadata::types::CapabilityType::Discovery,
        metadata::types::CapabilityType::Metadata};

    return meta;
  }
}

int main()
{
  try
  {
    log_info("initializing store...");
    store::engine::StoreEngine store(build_store_config());

    log_info("initializing sync...");
    sync::core::SyncConfig sync_config = build_sync_config();
    sync::core::SyncContext sync_context;
    sync_context.store = &store;
    sync_context.config = &sync_config;
    sync::engine::SyncEngine sync(sync_context);

    log_info("initializing transport...");
    transport::core::TransportConfig transport_config = build_transport_config();
    transport::core::TransportContext transport_context;
    transport_context.config = &transport_config;
    transport_context.sync = &sync;

    transport::backend::TcpTransportBackend transport_backend(transport_config);
    transport::engine::TransportEngine transport_engine(transport_context, transport_backend);

    if (!transport_engine.start())
    {
      std::cerr << "[metadata-registry] failed to start transport engine\n";
      return 1;
    }

    log_info("initializing discovery...");
    discovery::core::DiscoveryConfig discovery_config = build_discovery_config();
    discovery::core::DiscoveryContext discovery_context;
    discovery_context.config = &discovery_config;
    discovery_context.transport = &transport_engine;

    discovery::backend::UdpDiscoveryBackend discovery_backend(discovery_config);
    discovery::engine::DiscoveryEngine discovery_engine(discovery_context, discovery_backend);

    if (!discovery_engine.start())
    {
      std::cerr << "[metadata-registry] failed to start discovery engine\n";
      transport_engine.stop();
      return 1;
    }

    log_info("initializing metadata...");
    metadata::core::MetadataConfig metadata_config = build_metadata_config();
    metadata::core::MetadataContext metadata_context;
    metadata_context.config = &metadata_config;
    metadata_context.discovery = &discovery_engine;

    metadata::engine::MetadataEngine metadata_engine(metadata_context);

    if (!metadata_engine.start())
    {
      std::cerr << "[metadata-registry] failed to start metadata engine\n";
      discovery_engine.stop();
      transport_engine.stop();
      return 1;
    }

    const auto local = metadata_engine.local_metadata_or_refresh();
    if (!local.has_value())
    {
      std::cerr << "[metadata-registry] failed to build local metadata\n";
      metadata_engine.stop();
      discovery_engine.stop();
      transport_engine.stop();
      return 1;
    }

    metadata_engine.upsert(make_remote_metadata());

    print_registry(metadata_engine);

    metadata_engine.stop();
    discovery_engine.stop();
    transport_engine.stop();

    log_info("demo completed successfully");
    return 0;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "[metadata-registry] fatal exception: " << ex.what() << '\n';
    return 1;
  }
  catch (...)
  {
    std::cerr << "[metadata-registry] fatal unknown exception\n";
    return 1;
  }
}
