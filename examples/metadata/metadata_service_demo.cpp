/*
 * metadata_service_demo.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>
#include <softadastra/discovery/Discovery.hpp>
#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA SERVICE DEMO ==\n";

  const std::string wal_path = "metadata_service_demo.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("metadata-node");

  sync::core::SyncContext sync_context{
      store,
      sync_config};

  sync::engine::SyncEngine sync_engine{
      sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7300);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend transport_backend{
      transport_config};

  transport::engine::TransportEngine transport_engine{
      transport_context,
      transport_backend};

  if (!transport_engine.start())
  {
    std::cerr << "failed to start transport engine\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto discovery_config =
      discovery::core::DiscoveryConfig::local(
          "metadata-node",
          9500,
          7300);

  discovery::core::DiscoveryContext discovery_context{
      discovery_config,
      transport_engine};

  discovery::backend::UdpDiscoveryBackend discovery_backend{
      discovery_config};

  discovery::engine::DiscoveryEngine discovery_engine{
      discovery_context,
      discovery_backend};

  if (!discovery_engine.start())
  {
    std::cerr << "failed to start discovery engine\n";
    transport_engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto metadata_options =
      metadata::MetadataOptions::local(
          "metadata-node",
          "1.0.0");

  metadata::MetadataService metadata_service{
      metadata_options,
      discovery_engine};

  if (!metadata_service.start())
  {
    std::cerr << "failed to start metadata service\n";
    discovery_engine.stop();
    transport_engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto local =
      metadata_service.local_or_refresh();

  if (local.has_value())
  {
    std::cout << "local metadata node id: "
              << local->node_id()
              << "\n";

    std::cout << "hostname: "
              << local->runtime.hostname
              << "\n";

    std::cout << "os: "
              << local->runtime.os_name
              << "\n";

    std::cout << "capabilities: "
              << local->capabilities.size()
              << "\n";
  }

  std::cout << "registry entries: "
            << metadata_service.all().size()
            << "\n";

  metadata_service.stop();
  discovery_engine.stop();
  transport_engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
