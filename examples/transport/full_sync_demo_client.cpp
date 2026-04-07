/*
 * full_sync_demo_client.cpp
 *
 * End-to-end demo client for:
 * STORE + SYNC + TRANSPORT
 *
 * Role:
 * - Starts a local store engine
 * - Starts a sync engine
 * - Starts a transport engine on port 9001
 * - Connects to the server on port 9000
 * - Sends hello
 * - Creates one local PUT operation
 * - Converts it into a sync batch
 * - Sends the batch to the server
 * - Polls for a short time to process any network follow-up
 * - Prints local store state for observability
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <softadastra/store/core/Entry.hpp>
#include <softadastra/store/core/Operation.hpp>
#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/OperationType.hpp>
#include <softadastra/store/types/Value.hpp>
#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>
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
    std::cout << "[client] " << message << '\n';
  }

  void logError(const std::string &message)
  {
    std::cerr << "[client] " << message << '\n';
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
    config.node_id = "node-client";
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig buildTransportConfig()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = 9001;
    return config;
  }

  transport::core::PeerInfo buildServerPeer()
  {
    transport::core::PeerInfo peer;
    peer.node_id = "node-server";
    peer.host = "127.0.0.1";
    peer.port = 9000;
    return peer;
  }

  store::types::Value makeValue(const std::string &text)
  {
    store::types::Value value;
    value.data.assign(text.begin(), text.end());
    return value;
  }

  std::string valueToString(const store::types::Value &value)
  {
    return std::string(value.data.begin(), value.data.end());
  }

  store::core::Operation buildDemoOperation()
  {
    store::core::Operation operation;
    operation.type = store::types::OperationType::Put;
    operation.key = {"user:1"};
    operation.value = makeValue("alice");
    operation.timestamp = 1000;
    return operation;
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

  bool startTransportEngine(transport::engine::TransportEngine &engine)
  {
    if (!engine.start())
    {
      logError("failed to start transport engine");
      return false;
    }

    logInfo("transport engine started");
    logInfo("bind host: 0.0.0.0");
    logInfo("bind port: 9001");
    logInfo("node id: node-client");
    return true;
  }

  bool connectWithRetry(
      transport::engine::TransportEngine &engine,
      const transport::core::PeerInfo &server_peer)
  {
    using namespace std::chrono_literals;

    constexpr int max_attempts = 10;

    for (int attempt = 1; attempt <= max_attempts && g_running; ++attempt)
    {
      logInfo("connecting to " + server_peer.node_id +
              " at " + server_peer.host + ":" + std::to_string(server_peer.port) +
              " (attempt " + std::to_string(attempt) + "/" + std::to_string(max_attempts) + ")");

      if (engine.connect_peer(server_peer))
      {
        logInfo("connection established");
        return true;
      }

      std::this_thread::sleep_for(500ms);
    }

    logError("failed to connect to server after retries");
    return false;
  }

  void sendHello(transport::engine::TransportEngine &engine,
                 const transport::core::PeerInfo &server_peer)
  {
    engine.send_hello(server_peer);
    logInfo("hello sent");
  }

  bool submitAndSendLocalOperation(sync::engine::SyncEngine &sync,
                                   transport::engine::TransportEngine &engine,
                                   const transport::core::PeerInfo &server_peer,
                                   const store::engine::StoreEngine &store)
  {
    const store::core::Operation operation = buildDemoOperation();

    logInfo("creating local operation...");
    logInfo("operation type: PUT");
    logInfo("operation key: user:1");
    logInfo("operation value: alice");

    const auto sync_operation = sync.submit_local_operation(operation);

    logInfo("sync operation created: " + sync_operation.sync_id);
    printStoreState(store);

    auto batch = sync.next_batch();

    if (batch.empty())
    {
      logError("sync batch is empty");
      return false;
    }

    logInfo("sync batch size: " + std::to_string(batch.size()));

    const std::size_t sent_count = engine.send_sync_batch(server_peer, batch);

    if (sent_count == 0)
    {
      logError("send_sync_batch returned 0");
      return false;
    }

    logInfo("sent batch count: " + std::to_string(sent_count));
    return true;
  }

  void pollForFollowUpTraffic(transport::engine::TransportEngine &engine,
                              const store::engine::StoreEngine &store)
  {
    using namespace std::chrono_literals;

    constexpr int loop_count = 20;
    std::size_t previous_entry_count = store.entries().size();

    logInfo("polling for network follow-up...");

    for (int i = 0; i < loop_count && g_running; ++i)
    {
      engine.poll_many(10);

      const std::size_t current_entry_count = store.entries().size();
      if (current_entry_count != previous_entry_count)
      {
        logInfo("local store changed during polling");
        printStoreState(store);
        previous_entry_count = current_entry_count;
      }

      std::this_thread::sleep_for(200ms);
    }

    logInfo("client flow completed");
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

    const transport::core::PeerInfo server_peer = buildServerPeer();

    if (!connectWithRetry(engine, server_peer))
    {
      return 1;
    }

    sendHello(engine, server_peer);

    if (!submitAndSendLocalOperation(sync, engine, server_peer, store))
    {
      return 1;
    }

    pollForFollowUpTraffic(engine, store);

    logInfo("client stopped cleanly");
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
