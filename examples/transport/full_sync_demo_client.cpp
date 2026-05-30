/*
 * full_sync_demo_client.cpp
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
  std::cout << "== FULL SYNC ASYNC DEMO CLIENT ==\n";

  const std::string wal_path = "full_sync_client_store.wal";
  std::filesystem::remove(wal_path);

  vix::async::core::io_context async_context{};

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-client");

  sync::core::SyncContext sync_context{
      store,
      sync_config};

  sync::engine::SyncEngine sync_engine{
      sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7101);

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
    std::cerr << "failed to start async transport client\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  std::thread runner(
      [&async_context]()
      {
        async_context.run();
      });

  transport::core::PeerInfo server{
      "node-server",
      "127.0.0.1",
      7100};

  if (!engine.connect_peer(server))
  {
    std::cerr << "failed to connect to server\n";

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

  engine.send_hello(server);

  auto operation =
      store::core::Operation::put(
          store::types::Key{"message:1"},
          store::types::Value::from_string("hello server"));

  auto submitted =
      sync_engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "failed to submit operation\n";

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

  std::cout << "sync envelopes accepted: "
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
