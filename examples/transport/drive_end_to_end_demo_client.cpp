/*
 * drive_end_to_end_demo_client.cpp
 *
 * End-to-end demo client:
 * filesystem -> wal -> store -> sync -> transport
 *
 * This client:
 * - creates a real workspace on disk
 * - takes snapshots before/after filesystem mutations
 * - computes fs diffs
 * - appends each file event to WAL
 * - materializes the latest file content into the local store
 * - submits sync operations
 * - sends batches to the server
 * - prints local store state
 */

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <softadastra/core/time/Timestamp.hpp>

#include <softadastra/fs/events/FileEvent.hpp>
#include <softadastra/fs/path/Path.hpp>
#include <softadastra/fs/snapshot/SnapshotBuilder.hpp>
#include <softadastra/fs/snapshot/SnapshotDiff.hpp>
#include <softadastra/fs/types/FileEventType.hpp>

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

#include <softadastra/wal/core/WalConfig.hpp>
#include <softadastra/wal/core/WalRecord.hpp>
#include <softadastra/wal/types/WalRecordType.hpp>
#include <softadastra/wal/utils/FileEventSerializer.hpp>
#include <softadastra/wal/writer/WalWriter.hpp>

using namespace softadastra;

namespace
{
  using SnapshotMap = decltype(fs::snapshot::SnapshotBuilder::build(
                                   fs::path::Path::from(".").value())
                                   .value()
                                   .all());

  std::atomic<bool> g_running{true};

  constexpr const char *kNodeId = "node-client";
  constexpr int kPort = 9001;

  const std::string kDemoRoot = "/tmp/softadastra_drive_demo";
  const std::string kClientWorkspace = kDemoRoot + "/client_workspace";
  const std::string kClientWalPath = kDemoRoot + "/client_ops.wal";

  const std::string kHelloRelativePath = "docs/hello.txt";
  const std::string kReadmeRelativePath = "docs/readme.txt";

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

  std::uint64_t nowTimestamp()
  {
    return static_cast<std::uint64_t>(core::time::Timestamp::now().value());
  }

  struct ChangeInfo
  {
    fs::events::FileEvent event;
    std::string logical_key;
    std::filesystem::path absolute_path;
  };

  fs::types::FileEventType passThroughEventType(const fs::types::FileEventType type)
  {
    return type;
  }

  wal::types::WalRecordType toWalRecordType(const fs::types::FileEventType type)
  {
    switch (type)
    {
    case fs::types::FileEventType::Created:
      return wal::types::WalRecordType::Put;
    case fs::types::FileEventType::Updated:
      return wal::types::WalRecordType::Update;
    case fs::types::FileEventType::Deleted:
      return wal::types::WalRecordType::Delete;
    }

    return wal::types::WalRecordType::Put;
  }

  store::types::OperationType toStoreOperationType(const fs::types::FileEventType type)
  {
    switch (type)
    {
    case fs::types::FileEventType::Created:
    case fs::types::FileEventType::Updated:
      return store::types::OperationType::Put;
    case fs::types::FileEventType::Deleted:
      return store::types::OperationType::Delete;
    }

    return store::types::OperationType::Put;
  }

  std::string eventTypeToString(const fs::types::FileEventType type)
  {
    switch (type)
    {
    case fs::types::FileEventType::Created:
      return "Created";
    case fs::types::FileEventType::Updated:
      return "Updated";
    case fs::types::FileEventType::Deleted:
      return "Deleted";
    }

    return "Unknown";
  }

  void ensureDemoDirectories()
  {
    std::filesystem::create_directories(kDemoRoot);
    std::filesystem::create_directories(kClientWorkspace);
    std::filesystem::create_directories(kClientWorkspace + "/docs");
  }

  void resetWorkspace()
  {
    std::filesystem::remove_all(kClientWorkspace);
    std::filesystem::create_directories(kClientWorkspace);
    std::filesystem::create_directories(kClientWorkspace + "/docs");
  }

  fs::path::Path requireRootPath(const std::string &raw_path)
  {
    auto root_result = fs::path::Path::from(raw_path);

    if (root_result.is_err())
    {
      std::cerr << "[client] invalid path: " << raw_path << '\n';
      std::exit(1);
    }

    return root_result.value();
  }

  auto buildSnapshotOrExit(const fs::path::Path &root, const std::string &label)
  {
    auto result = fs::snapshot::SnapshotBuilder::build(root);

    if (result.is_err())
    {
      std::cerr << "[client] snapshot error (" << label << "): "
                << result.error().message()
                << '\n';
      std::exit(1);
    }

    return result.value();
  }

  store::types::Key makeStoreKey(const std::string &value)
  {
    store::types::Key key;
    key.value = value;
    return key;
  }

  store::types::Value makeStoreValue(const std::vector<std::uint8_t> &payload)
  {
    store::types::Value value;
    value.data = payload;
    return value;
  }

  std::string valueToString(const store::types::Value &value)
  {
    return std::string(value.data.begin(), value.data.end());
  }

  std::vector<std::uint8_t> readFileBytes(const std::filesystem::path &path)
  {
    std::ifstream input(path, std::ios::binary);

    if (!input)
    {
      return {};
    }

    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
  }

  void writeTextFile(const std::filesystem::path &path, const std::string &content)
  {
    std::filesystem::create_directories(path.parent_path());

    std::ofstream output(path, std::ios::binary);
    output << content;
  }

  std::string makeLogicalKey(
      const std::string &workspace_root,
      const std::string &raw_path)
  {
    const std::filesystem::path root(workspace_root);
    const std::filesystem::path absolute(raw_path);

    const auto relative = std::filesystem::relative(absolute, root);
    return std::string("demo/") + relative.generic_string();
  }

  std::vector<ChangeInfo> buildChanges(
      const SnapshotMap &before,
      const SnapshotMap &after,
      const std::string &workspace_root)
  {
    std::vector<ChangeInfo> changes;

    const auto diff = fs::snapshot::SnapshotDiff::compute(before, after);

    for (const auto &change : diff)
    {
      fs::events::FileEvent event{
          passThroughEventType(change.type),
          change.current,
          std::nullopt};

      const std::string raw_path = event.current.path.str();

      changes.push_back(ChangeInfo{
          event,
          makeLogicalKey(workspace_root, raw_path),
          std::filesystem::path(raw_path)});
    }

    return changes;
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

  transport::core::PeerInfo buildServerPeer()
  {
    transport::core::PeerInfo peer;
    peer.node_id = "node-server";
    peer.host = "127.0.0.1";
    peer.port = 9000;
    return peer;
  }

  wal::writer::WalWriter buildWalWriter()
  {
    wal::core::WalConfig config;
    config.path = kClientWalPath;
    config.auto_flush = true;
    return wal::writer::WalWriter(config);
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
    logInfo("client wal: " + kClientWalPath);
    logInfo("workspace: " + kClientWorkspace);
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

  void sendHello(
      transport::engine::TransportEngine &engine,
      const transport::core::PeerInfo &server_peer)
  {
    engine.send_hello(server_peer);
    logInfo("hello sent");
  }

  bool sendPendingBatches(
      sync::engine::SyncEngine &sync,
      transport::engine::TransportEngine &engine,
      const transport::core::PeerInfo &server_peer)
  {
    bool sent_anything = false;

    for (int i = 0; i < 128; ++i)
    {
      auto batch = sync.next_batch();

      if (batch.empty())
      {
        break;
      }

      logInfo("sync batch size: " + std::to_string(batch.size()));

      const std::size_t sent_count = engine.send_sync_batch(server_peer, batch);

      if (sent_count == 0)
      {
        logError("send_sync_batch returned 0");
        return false;
      }

      sent_anything = true;
      logInfo("sent batch count: " + std::to_string(sent_count));
    }

    if (!sent_anything)
    {
      logError("no sync batch was produced");
      return false;
    }

    return true;
  }

  bool isRegularFileChange(const ChangeInfo &change)
  {
    namespace fsys = std::filesystem;

    switch (change.event.type)
    {
    case fs::types::FileEventType::Created:
    case fs::types::FileEventType::Updated:
      return fsys::exists(change.absolute_path) &&
             fsys::is_regular_file(change.absolute_path);

    case fs::types::FileEventType::Deleted:
      return true;
    }

    return false;
  }

  bool applyFsChange(
      const ChangeInfo &change,
      wal::writer::WalWriter &writer,
      store::engine::StoreEngine &store,
      sync::engine::SyncEngine &sync)
  {
    if (!isRegularFileChange(change))
    {
      logInfo("skipping non-regular file change: " +
              eventTypeToString(change.event.type) +
              " key=" + change.logical_key);
      return true;
    }

    logInfo("processing fs change: " + eventTypeToString(change.event.type) +
            " key=" + change.logical_key);

    wal::core::WalRecord record;
    record.type = toWalRecordType(change.event.type);
    record.timestamp = nowTimestamp();
    record.payload = wal::utils::FileEventSerializer::serialize(change.event);

    const auto wal_seq = writer.append(record);

    std::cout << "[client] wal append seq=" << wal_seq
              << " type=" << eventTypeToString(change.event.type)
              << " key=" << change.logical_key
              << '\n';

    store::core::Operation op;
    op.type = toStoreOperationType(change.event.type);
    op.key = makeStoreKey(change.logical_key);
    op.timestamp = record.timestamp;

    switch (change.event.type)
    {
    case fs::types::FileEventType::Created:
    case fs::types::FileEventType::Updated:
    {
      const auto bytes = readFileBytes(change.absolute_path);
      const auto value = makeStoreValue(bytes);

      const auto put_result = store.put(op.key, value);
      if (!put_result.success)
      {
        logError("local store put failed for key=" + change.logical_key);
        return false;
      }

      op.value = value;
      const auto sync_op = sync.submit_local_operation(op);

      logInfo("sync operation created: " + sync_op.sync_id);
      return true;
    }

    case fs::types::FileEventType::Deleted:
    {
      const auto remove_result = store.remove(op.key);
      if (!remove_result.success)
      {
        logError("local store remove failed for key=" + change.logical_key);
        return false;
      }

      const auto sync_op = sync.submit_local_operation(op);
      logInfo("sync operation created: " + sync_op.sync_id);
      return true;
    }
    }

    return false;
  }

  bool processSnapshotDiff(
      const SnapshotMap &before,
      const SnapshotMap &after,
      const std::string &workspace_root,
      wal::writer::WalWriter &writer,
      store::engine::StoreEngine &store,
      sync::engine::SyncEngine &sync,
      transport::engine::TransportEngine &engine,
      const transport::core::PeerInfo &server_peer)
  {
    const auto changes = buildChanges(before, after, workspace_root);

    if (changes.empty())
    {
      logInfo("no filesystem changes detected");
      return true;
    }

    logInfo("detected " + std::to_string(changes.size()) + " filesystem change(s)");

    for (const auto &change : changes)
    {
      if (!applyFsChange(change, writer, store, sync))
      {
        return false;
      }
    }

    printStoreState(store);
    return sendPendingBatches(sync, engine, server_peer);
  }

  void pumpNetworkBriefly(transport::engine::TransportEngine &engine, int rounds)
  {
    using namespace std::chrono_literals;

    for (int i = 0; i < rounds && g_running; ++i)
    {
      engine.poll_many(1);
      std::this_thread::sleep_for(50ms);
    }
  }

  bool runFilesystemScenario(
      wal::writer::WalWriter &writer,
      store::engine::StoreEngine &store,
      sync::engine::SyncEngine &sync,
      transport::engine::TransportEngine &engine,
      const transport::core::PeerInfo &server_peer)
  {
    using namespace std::chrono_literals;

    logInfo("resetting workspace...");
    resetWorkspace();

    const auto root = requireRootPath(kClientWorkspace);
    logInfo("workspace ready");

    logInfo("scenario step 1: create hello.txt");
    const auto before_step1 = buildSnapshotOrExit(root, "before-step1").all();

    writeTextFile(
        std::filesystem::path(kClientWorkspace) / kHelloRelativePath,
        "hello from node-client v1\n");

    const auto after_step1 = buildSnapshotOrExit(root, "after-step1").all();

    logInfo("processing diff for step 1...");
    if (!processSnapshotDiff(
            before_step1,
            after_step1,
            kClientWorkspace,
            writer,
            store,
            sync,
            engine,
            server_peer))
    {
      logError("step 1 failed");
      return false;
    }

    logInfo("step 1 sent successfully");
    logInfo("brief network pump after step 1...");
    pumpNetworkBriefly(engine, 3);

    std::this_thread::sleep_for(200ms);

    logInfo("scenario step 2: update hello.txt and create readme.txt");
    const auto before_step2 = buildSnapshotOrExit(root, "before-step2").all();

    writeTextFile(
        std::filesystem::path(kClientWorkspace) / kHelloRelativePath,
        "hello from node-client v2\n");

    writeTextFile(
        std::filesystem::path(kClientWorkspace) / kReadmeRelativePath,
        "replicated from fs diff\n");

    const auto after_step2 = buildSnapshotOrExit(root, "after-step2").all();

    logInfo("processing diff for step 2...");
    if (!processSnapshotDiff(
            before_step2,
            after_step2,
            kClientWorkspace,
            writer,
            store,
            sync,
            engine,
            server_peer))
    {
      logError("step 2 failed");
      return false;
    }

    logInfo("step 2 sent successfully");
    logInfo("final brief network pump...");
    pumpNetworkBriefly(engine, 5);

    logInfo("filesystem scenario completed");
    printStoreState(store);
    return true;
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

    logInfo("initializing wal...");
    auto writer = buildWalWriter();

    if (!startTransportEngine(engine))
    {
      return 1;
    }

    const auto server_peer = buildServerPeer();

    if (!connectWithRetry(engine, server_peer))
    {
      return 1;
    }

    sendHello(engine, server_peer);

    if (!runFilesystemScenario(writer, store, sync, engine, server_peer))
    {
      return 1;
    }

    logInfo("drive end-to-end demo completed successfully");
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
