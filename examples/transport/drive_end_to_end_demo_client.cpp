/*
 * drive_end_to_end_demo_client.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DRIVE END TO END DEMO CLIENT ==\n";

  const std::string wal_path = "drive_client_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("drive-client");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7201);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend backend{transport_config};
  transport::engine::TransportEngine engine{
      transport_context,
      backend};

  if (!engine.start())
  {
    std::cerr << "failed to start drive client\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  transport::core::PeerInfo server{
      "drive-server",
      "127.0.0.1",
      7200};

  if (!engine.connect_peer(server))
  {
    std::cerr << "failed to connect to drive server\n";
    engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto file_operation = store::core::Operation::put(
      store::types::Key{"files/docs/readme.txt"},
      store::types::Value::from_string("offline-first file content"));

  auto submitted =
      sync_engine.submit_local_operation(file_operation);

  if (submitted.is_err())
  {
    std::cerr << "failed to submit drive operation\n";
    engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto batch =
      sync_engine.next_batch();

  const auto sent =
      engine.send_sync_batch(server, batch);

  std::cout << "drive sync messages sent: "
            << sent
            << "\n";

  engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
