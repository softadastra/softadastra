/*
 * discovery_service.cpp
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
  std::cout << "== DISCOVERY SERVICE EXAMPLE ==\n";

  const std::string wal_path = "discovery_service_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-a");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7000);

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

  auto options =
      discovery::DiscoveryOptions::local(
          "node-a",
          9400,
          7000);

  discovery::DiscoveryService service{
      options,
      transport_engine};

  service.on_peer_found(
      [](const discovery::Peer &peer)
      {
        std::cout << "peer found: "
                  << peer.node_id
                  << " "
                  << peer.host
                  << ":"
                  << peer.port
                  << "\n";
      });

  if (!service.start())
  {
    std::cerr << "failed to start discovery service\n";
    transport_engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  service.announce_now();
  service.probe_now();
  service.poll_many(4);

  std::cout << "known peers: "
            << service.peers().size()
            << "\n";

  service.stop();
  transport_engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
