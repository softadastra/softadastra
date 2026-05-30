/*
 * full_sync_demo_server.cpp
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
  std::cout << "== FULL SYNC ASYNC DEMO SERVER ==\n";

  const std::string wal_path = "full_sync_server_store.wal";
  std::filesystem::remove(wal_path);

  vix::async::core::io_context async_context{};

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-server");

  sync::core::SyncContext sync_context{
      store,
      sync_config};

  sync::engine::SyncEngine sync_engine{
      sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7100);

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
    std::cerr << "failed to start async transport server\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  std::thread runner(
      [&async_context]()
      {
        async_context.run();
      });

  std::cout << "server started on 127.0.0.1:7100\n";
  std::cout << "waiting for sync messages...\n";

  std::size_t processed_events = 0;

  for (std::size_t tick = 0; tick < 50; ++tick)
  {
    std::this_thread::sleep_for(
        std::chrono::milliseconds{100});

    const auto events =
        backend.drain_events(128);

    if (!events.empty())
    {
      processed_events +=
          engine.process_events(events);
    }

    if (!store.entries().empty())
    {
      break;
    }
  }

  std::cout << "processed transport events: "
            << processed_events
            << "\n";

  std::cout << "store entries: "
            << store.entries().size()
            << "\n";

  engine.stop();
  async_context.stop();

  if (runner.joinable())
  {
    runner.join();
  }

  std::filesystem::remove(wal_path);

  return store.entries().empty() ? 1 : 0;
}
