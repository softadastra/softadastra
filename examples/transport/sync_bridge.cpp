/*
 * sync_bridge.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT SYNC BRIDGE EXAMPLE ==\n";

  const std::string wal_path = "sync_bridge_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-a");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto operation = store::core::Operation::put(
      store::types::Key{"doc:1"},
      store::types::Value::from_string("hello from node-a"));

  auto submitted =
      sync_engine.submit_local_operation(operation);

  if (submitted.is_err())
  {
    std::cerr << "failed to submit local operation\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto batch =
      sync_engine.next_batch();

  if (batch.empty())
  {
    std::cerr << "no sync envelope ready\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  auto payload =
      transport::dispatcher::MessageDispatcher::encode_sync_operation(
          batch.front().operation);

  auto message =
      transport::core::TransportMessage::sync_batch(
          "node-a",
          payload);

  message.to_node_id = "node-b";
  message.correlation_id = batch.front().operation.sync_id;

  auto frame =
      transport::encoding::MessageEncoder::encode_frame(message);

  std::cout << "sync id: "
            << batch.front().operation.sync_id
            << "\n";

  std::cout << "encoded transport frame size: "
            << frame.size()
            << "\n";

  std::filesystem::remove(wal_path);

  return 0;
}
