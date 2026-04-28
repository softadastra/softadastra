/*
 * drive_end_to_end_demo_server.cpp
 *
 * End-to-end demo server:
 * filesystem -> wal -> store -> sync -> transport -> store -> wal(audit)
 *
 * This server:
 * - starts store + sync + transport
 * - polls incoming network traffic
 * - observes store changes
 * - appends an audit WAL locally for each observed store mutation
 * - prints converged state
 * - exits automatically when the expected final state is reached
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include <softadastra/core/time/Timestamp.hpp>

#include <softadastra/store/core/Entry.hpp>
#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>
#include <softadastra/store/types/Key.hpp>
#include <softadastra/store/types/Value.hpp>

#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>

#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

namespace
{
  std::atomic<bool> g_running{true};

  constexpr const char *kNodeId = "node-server";
  constexpr int kPort = 9000;

  const std::string kDemoRoot = "/tmp/softadastra_drive_demo";
  const std::string kServerWalPath = kDemoRoot + "/server_audit.wal";

  const std::string kHelloKey = "demo/docs/hello.txt";
  const std::string kReadmeKey = "demo/docs/readme.txt";

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

  std::uint64_t nowTimestamp()
  {
    return static_cast<std::uint64_t>(core::time::Timestamp::now().value());
  }

  void ensureDemoDirectories()
  {
    std::filesystem::create_directories(kDemoRoot);
  }

  store::types::Key makeStoreKey(const std::string &value)
  {
    store::types::Key key;
    key.value = value;
    return key;
  }

  std::string valueToString(const store::types::Value &value)
  {
    return std::string(value.data.begin(), value.data.end());
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
    config.node_id = kNodeId;
    config.auto_queue = true;
    config.require_ack = true;
    return config;
  }

  transport::core::TransportConfig buildTransportConfig()
  {
    transport::core::TransportConfig config;
    config.bind_host = "0.0.0.0";
    config.bind_port = kPort;
    return config;
  }

  wal::writer::WalWriter buildAuditWalWriter()
  {
    wal::core::WalConfig config;
    config.path = kServerWalPath;
    config.auto_flush = true;
    return wal::writer::WalWriter(config);
  }

  bool valuesEqual(const store::types::Value &a, const store::types::Value &b)
  {
    return a.data == b.data;
  }

  bool entriesEqual(const store::core::Entry &a, const store::core::Entry &b)
  {
    return valuesEqual(a.value, b.value) &&
           a.version == b.version &&
           a.timestamp == b.timestamp;
  }

  void printStoreState(const store::engine::StoreEngine &store)
  {
    const auto &entries = store.entries();

    if (entries.empty())
    {
      logInfo("store state: empty");
      return;
    }

    logInfo("store state:");

    for (const auto &[key, entry] : entries)
    {
      std::cout << "  - key=" << key
                << ", value=" << valueToString(entry.value)
                << ", version=" << entry.version
                << ", timestamp=" << entry.timestamp
                << '\n';
    }
  }

  wal::types::WalRecordType inferWalType(
      const std::unordered_map<std::string, store::core::Entry> &previous,
      const std::string &key)
  {
    return previous.contains(key)
               ? wal::types::WalRecordType::Update
               : wal::types::WalRecordType::Put;
  }

  void appendAuditRecordForPut(
      wal::writer::WalWriter &writer,
      const std::unordered_map<std::string, store::core::Entry> &previous,
      const std::string &key,
      const store::core::Entry &entry)
  {
    wal::core::WalRecord record;
    record.type = inferWalType(previous, key);
    record.timestamp = nowTimestamp();
    record.payload = entry.value.data;

    const auto seq = writer.append(record);

    std::cout << "[server] audit wal append seq=" << seq
              << " type=Put"
              << " key=" << key
              << '\n';
  }

  void appendAuditRecordForDelete(
      wal::writer::WalWriter &writer,
      const std::string &key)
  {
    wal::core::WalRecord record;
    record.type = wal::types::WalRecordType::Delete;
    record.timestamp = nowTimestamp();

    const auto seq = writer.append(record);

    std::cout << "[server] audit wal append seq=" << seq
              << " type=Delete"
              << " key=" << key
              << '\n';
  }

  void observeAndAuditStoreChanges(
      const store::engine::StoreEngine &store,
      wal::writer::WalWriter &writer,
      std::unordered_map<std::string, store::core::Entry> &previous_entries)
  {
    const auto &current_entries = store.entries();
    bool changed = false;

    for (const auto &[key, entry] : current_entries)
    {
      const auto it = previous_entries.find(key);

      if (it == previous_entries.end() || !entriesEqual(it->second, entry))
      {
        appendAuditRecordForPut(writer, previous_entries, key, entry);
        changed = true;
      }
    }

    for (const auto &[key, _] : previous_entries)
    {
      if (!current_entries.contains(key))
      {
        appendAuditRecordForDelete(writer, key);
        changed = true;
      }
    }

    if (changed)
    {
      logInfo("store changed");
      printStoreState(store);
      previous_entries = current_entries;
    }
  }

  std::optional<std::string> getStringValue(
      const store::engine::StoreEngine &store,
      const std::string &key)
  {
    const auto entry = store.get(makeStoreKey(key));

    if (!entry.has_value())
    {
      return std::nullopt;
    }

    return valueToString(entry->value);
  }

  bool expectedFinalStateReached(const store::engine::StoreEngine &store)
  {
    const auto hello = getStringValue(store, kHelloKey);
    const auto readme = getStringValue(store, kReadmeKey);

    return hello.has_value() &&
           readme.has_value() &&
           *hello == "hello from node-client v2\n" &&
           *readme == "replicated from fs diff\n";
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
    logInfo("audit wal: " + kServerWalPath);
    logInfo("waiting for replicated state...");
    return true;
  }

  void runServerLoop(
      transport::engine::TransportEngine &engine,
      const store::engine::StoreEngine &store,
      wal::writer::WalWriter &audit_writer)
  {
    using namespace std::chrono_literals;

    std::unordered_map<std::string, store::core::Entry> previous_entries;
    std::size_t tick_count = 0;
    bool convergence_announced = false;
    std::size_t stable_ticks_after_convergence = 0;

    while (g_running)
    {
      engine.poll_many(10);
      observeAndAuditStoreChanges(store, audit_writer, previous_entries);

      if (expectedFinalStateReached(store))
      {
        if (!convergence_announced)
        {
          logInfo("expected final state reached");
          printStoreState(store);
          convergence_announced = true;
        }

        ++stable_ticks_after_convergence;
        if (stable_ticks_after_convergence >= 10)
        {
          logInfo("server demo completed successfully");
          break;
        }
      }

      ++tick_count;
      if (tick_count % 25 == 0)
      {
        logInfo("polling transport...");
      }

      std::this_thread::sleep_for(200ms);
    }
  }
}

int main()
{
  try
  {
    installSignalHandlers();
    ensureDemoDirectories();

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

    logInfo("initializing audit wal...");
    auto audit_writer = buildAuditWalWriter();

    if (!startTransportEngine(engine))
    {
      return 1;
    }

    runServerLoop(engine, store, audit_writer);

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
