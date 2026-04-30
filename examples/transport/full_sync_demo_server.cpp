/*
 * full_sync_demo_server.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== FULL SYNC DEMO SERVER ==\n";

  const std::string wal_path = "full_sync_server_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-server");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7100);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend backend{transport_config};
  transport::engine::TransportEngine engine{
      transport_context,
      backend};

  if (!engine.start())
  {
    std::cerr << "failed to start transport server\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  std::cout << "server started on 127.0.0.1:7100\n";
  std::cout << "polling one message...\n";

  const bool handled = engine.poll_once();

  std::cout << "handled: "
            << handled
            << "\n";

  engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
