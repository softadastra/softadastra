/*
 * drive_end_to_end_demo_server.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DRIVE END TO END DEMO SERVER ==\n";

  const std::string wal_path = "drive_server_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("drive-server");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7200);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend backend{transport_config};
  transport::engine::TransportEngine engine{
      transport_context,
      backend};

  if (!engine.start())
  {
    std::cerr << "failed to start drive server\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  std::cout << "drive server ready on 127.0.0.1:7200\n";

  const auto processed =
      engine.poll_many(8);

  std::cout << "processed messages: "
            << processed
            << "\n";

  std::cout << "store entries: "
            << store.entries().size()
            << "\n";

  engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
