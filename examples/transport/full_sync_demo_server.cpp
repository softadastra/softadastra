/*
 * full_sync_demo_server.cpp
 *
 * End-to-end demo server for:
 * STORE + SYNC + TRANSPORT
 *
 * Role:
 * - Starts a local store engine
 * - Starts a sync engine
 * - Starts a transport engine on port 9000
 * - Polls continuously to receive sync traffic
 * - Prints current store state periodically
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <string>
#include <thread>

#include <softadastra/store/core/Entry.hpp>
#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

using namespace softadastra;

namespace
{
  std::atomic<bool> g_running{true};

  void handleSignal(int)
  {
    g_running = false;
  }

  void installSignalHandlers()
  {
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);
  }

  void logInfo(const std::string &message)
  {
    std::cout << "[server] " << message << '\n';
  }

  void logError(const std::string &message)
  {
    std::cerr << "[server] " << message << '\n';
  }

  store::core::StoreConfig buildStoreConfig()
  {
    store::core::StoreConfig config;
    config.enable_wal = false;
    return config;
  }

  sync::core::SyncConfig buildSyncConfig()
  {
    sync::core::SyncConfig config;
    config.node_id = "node-server";
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig buildTransportConfig()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = 9000;
    return config;
  }

  bool startTransportEngine(transport::engine::TransportEngine &engine)
  {
    if (!engine.start())
    {
      logError("failed to start transport engine");
      return false;
    }

    logInfo("transport engine started");
    logInfo("bind host: 0.0.0.0");
    logInfo("bind port: 9000");
    logInfo("node id: node-server");
    logInfo("waiting for sync traffic...");
    return true;
  }

  std::string valueToString(const store::types::Value &value)
  {
    return std::string(value.data.begin(), value.data.end());
  }

  void printStoreState(const store::engine::StoreEngine &store)
  {
    const auto &all_entries = store.entries();

    if (all_entries.empty())
    {
      logInfo("store state: empty");
      return;
    }

    logInfo("store state:");

    for (const auto &[key, entry] : all_entries)
    {
      std::cout << "  - key=" << key
                << ", value=" << valueToString(entry.value)
                << ", version=" << entry.version
                << ", timestamp=" << entry.timestamp
                << '\n';
    }
  }

  void runPollingLoop(transport::engine::TransportEngine &engine,
                      const store::engine::StoreEngine &store)
  {
    using namespace std::chrono_literals;

    std::size_t tick_count = 0;
    std::size_t previous_entry_count = 0;
    constexpr std::size_t status_interval = 25;

    while (g_running)
    {
      engine.poll_many(10);
      ++tick_count;

      const std::size_t current_entry_count = store.entries().size();

      if (current_entry_count != previous_entry_count)
      {
        logInfo("store changed");
        printStoreState(store);
        previous_entry_count = current_entry_count;
      }

      if (tick_count % status_interval == 0)
      {
        logInfo("polling transport...");
        printStoreState(store);
      }

      std::this_thread::sleep_for(200ms);
    }

    logInfo("shutdown requested");
    printStoreState(store);
  }
}

int main()
{
  try
  {
    installSignalHandlers();

    logInfo("initializing store...");
    store::engine::StoreEngine store(buildStoreConfig());

    logInfo("initializing sync...");
    sync::core::SyncConfig sync_config = buildSyncConfig();
    sync::core::SyncContext sync_context;
    sync_context.store = &store;
    sync_context.config = &sync_config;

    sync::engine::SyncEngine sync(sync_context);

    logInfo("initializing transport...");
    transport::core::TransportConfig transport_config = buildTransportConfig();
    transport::core::TransportContext transport_context;
    transport_context.config = &transport_config;
    transport_context.sync = &sync;

    transport::backend::TcpTransportBackend backend(transport_config);
    transport::engine::TransportEngine engine(transport_context, backend);

    if (!startTransportEngine(engine))
    {
      return 1;
    }

    runPollingLoop(engine, store);

    logInfo("server stopped cleanly");
    return 0;
  }
  catch (const std::exception &ex)
  {
    logError(std::string("fatal exception: ") + ex.what());
    return 1;
  }
  catch (...)
  {
    logError("fatal unknown exception");
    return 1;
  }
}
