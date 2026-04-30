/*
 * discovery_engine_setup.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>
#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY ENGINE SETUP EXAMPLE ==\n";

  const std::string wal_path = "discovery_engine_setup_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-engine");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7010);

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
          "node-engine",
          9410,
          7010);

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

  discovery_engine.announce_now();
  discovery_engine.probe_now();

  const auto processed =
      discovery_engine.poll_many(4);

  std::cout << "processed discovery messages: "
            << processed
            << "\n";

  std::cout << "available transport peers: "
            << discovery_engine.available_transport_peers().size()
            << "\n";

  discovery_engine.stop();
  transport_engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
