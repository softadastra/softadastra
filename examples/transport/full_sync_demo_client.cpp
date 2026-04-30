/*
 * full_sync_demo_client.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== FULL SYNC DEMO CLIENT ==\n";

  const std::string wal_path = "full_sync_client_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-client");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7101);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::TcpTransportBackend backend{transport_config};
  transport::engine::TransportEngine engine{
      transport_context,
      backend};

  if (!engine.start())
  {
    std::cerr << "failed to start transport client\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  transport::core::PeerInfo server{
      "node-server",
      "127.0.0.1",
      7100};

  if (!engine.connect_peer(server))
  {
    std::cerr << "failed to connect to server\n";
    engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  engine.send_hello(server);

  auto operation = store::core::Operation::put(
      store::types::Key{"message:1"},
      store::types::Value::from_string("hello server"));

  auto submitted =
      sync_engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "failed to submit operation\n";
    engine.stop();
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto batch =
      sync_engine.next_batch();

  const auto sent =
      engine.send_sync_batch(server, batch);

  std::cout << "sync envelopes sent: "
            << sent
            << "\n";

  engine.stop();

  std::filesystem::remove(wal_path);

  return 0;
}
