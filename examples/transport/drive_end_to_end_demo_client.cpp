/*
 * drive_end_to_end_demo_client.cpp
 */

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <thread>

#include <vix/async/core/io_context.hpp>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DRIVE END TO END DEMO ASYNC CLIENT ==\n";

  const std::string wal_path = "drive_client_store.wal";
  std::filesystem::remove(wal_path);

  vix::async::core::io_context async_context{};

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("drive-client");

  sync::core::SyncContext sync_context{
      store,
      sync_config};

  sync::engine::SyncEngine sync_engine{
      sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7201);

  transport_config.enable_keepalive = false;

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::backend::AsyncTcpTransportBackend backend{
      async_context,
      transport_config};

  transport::engine::TransportEngine engine{
      transport_context,
      backend};

  if (!engine.start())
  {
    std::cerr << "failed to start drive async client\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  std::thread runner(
      [&async_context]()
      {
        async_context.run();
      });

  transport::core::PeerInfo server{
      "drive-server",
      "127.0.0.1",
      7200};

  if (!engine.connect_peer(server))
  {
    std::cerr << "failed to connect to drive server\n";

    engine.stop();
    async_context.stop();

    if (runner.joinable())
    {
      runner.join();
    }

    std::filesystem::remove(wal_path);
    return 1;
  }

  std::this_thread::sleep_for(
      std::chrono::milliseconds{300});

  engine.process_events(
      backend.drain_events(64));

  auto file_operation =
      store::core::Operation::put(
          store::types::Key{"files/docs/readme.txt"},
          store::types::Value::from_string(
              "offline-first file content"));

  auto submitted =
      sync_engine.submit_local_operation(file_operation);

  if (submitted.is_err())
  {
    std::cerr << "failed to submit drive operation\n";

    engine.stop();
    async_context.stop();

    if (runner.joinable())
    {
      runner.join();
    }

    std::filesystem::remove(wal_path);
    return 1;
  }

  auto batch =
      sync_engine.next_batch();

  const auto sent =
      engine.send_sync_batch(server, batch);

  std::this_thread::sleep_for(
      std::chrono::milliseconds{500});

  const auto processed_events =
      engine.process_events(
          backend.drain_events(128));

  std::cout << "drive sync messages accepted: "
            << sent
            << "\n";

  std::cout << "transport events processed: "
            << processed_events
            << "\n";

  engine.stop();
  async_context.stop();

  if (runner.joinable())
  {
    runner.join();
  }

  std::filesystem::remove(wal_path);

  return sent > 0 ? 0 : 1;
}
