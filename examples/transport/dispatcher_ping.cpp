/*
 * dispatcher_ping.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT DISPATCHER PING EXAMPLE ==\n";

  const std::string wal_path = "dispatcher_ping_store.wal";
  std::filesystem::remove(wal_path);

  store::engine::StoreEngine store{
      store::core::StoreConfig::durable(wal_path)};

  auto sync_config =
      sync::core::SyncConfig::durable("node-local");

  sync::core::SyncContext sync_context{store, sync_config};
  sync::engine::SyncEngine sync_engine{sync_context};

  auto transport_config =
      transport::core::TransportConfig::local(7000);

  transport::core::TransportContext transport_context{
      transport_config,
      sync_engine};

  transport::dispatcher::MessageDispatcher dispatcher{
      transport_context};

  auto ping =
      transport::core::TransportMessage::ping("node-remote");

  ping.to_node_id = "node-local";
  ping.correlation_id = "ping-1";

  auto result = dispatcher.dispatch(ping);

  if (result.is_err())
  {
    std::cerr << "dispatch failed: "
              << result.error().message()
              << "\n";
    std::filesystem::remove(wal_path);
    return 1;
  }

  if (result.value().reply.has_value())
  {
    std::cout << "reply type: "
              << transport::types::to_string(result.value().reply->type)
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
