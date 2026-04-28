/*
 * SoftadastraRuntime.cpp
 */

#include "runtime/SoftadastraRuntime.hpp"
#include "commands/node/NodeInfoCommand.hpp"
#include "commands/node/NodeStartCommand.hpp"
#include "commands/peers/PeersCommand.hpp"
#include "commands/status/StatusCommand.hpp"
#include "commands/store/StoreGetCommand.hpp"
#include "commands/store/StorePutCommand.hpp"
#include "commands/sync/SyncStatusCommand.hpp"
#include "commands/sync/SyncTickCommand.hpp"

#include <memory>
#include <filesystem>
#include <utility>

namespace softadastra::app::cli
{
  namespace fs = std::filesystem;

  SoftadastraRuntime::SoftadastraRuntime(const cli_core::CliConfig &cli_config)
      : cli_config_(cli_config),
        store_config_(make_store_config()),
        store_(store_config_),
        sync_config_(make_sync_config()),
        sync_context_{},
        sync_([&]() -> sync_core::SyncContext &
              {
                sync_context_.store = &store_;
                sync_context_.config = &sync_config_;
                return sync_context_; }()),
        transport_config_(make_transport_config()),
        transport_context_{},
        transport_backend_(transport_config_),
        transport_([&]() -> transport_core::TransportContext &
                   {
                     transport_context_.config = &transport_config_;
                     transport_context_.sync = &sync_;
                     return transport_context_; }(),
                   transport_backend_),
        discovery_options_(make_discovery_options(sync_config_.node_id)),
        discovery_(discovery_options_, transport_),
        metadata_options_(make_metadata_options(sync_config_.node_id,
                                                cli_config_.version)),
        metadata_(metadata_options_, discovery_.engine()),
        cli_(cli_config_),
        valid_(false),
        node_running_(false)
  {
    valid_ =
        cli_config_.valid() &&
        sync_context_.valid() &&
        transport_context_.valid() &&
        discovery_options_.valid() &&
        metadata_options_.valid();

    if (valid_)
    {
      register_commands();
    }
  }

  SoftadastraRuntime::~SoftadastraRuntime()
  {
    stop_node();
  }

  bool SoftadastraRuntime::valid() const noexcept
  {
    return valid_;
  }

  bool SoftadastraRuntime::start_node()
  {
    if (!valid_)
    {
      return false;
    }

    if (node_running_)
    {
      return true;
    }

    if (!transport_.start())
    {
      return false;
    }

    if (!discovery_.start())
    {
      transport_.stop();
      return false;
    }

    if (!metadata_.start())
    {
      discovery_.stop();
      transport_.stop();
      return false;
    }

    node_running_ = true;
    return true;
  }

  void SoftadastraRuntime::stop_node()
  {
    if (!node_running_)
    {
      return;
    }

    metadata_.stop();
    discovery_.stop();
    transport_.stop();

    node_running_ = false;
  }

  bool SoftadastraRuntime::node_running() const noexcept
  {
    return node_running_;
  }

  const std::string &SoftadastraRuntime::node_id() const
  {
    return sync_config_.node_id;
  }

  cli_service::CliService &SoftadastraRuntime::cli() noexcept
  {
    return cli_;
  }

  const cli_service::CliService &SoftadastraRuntime::cli() const noexcept
  {
    return cli_;
  }

  store_engine::StoreEngine &SoftadastraRuntime::store() noexcept
  {
    return store_;
  }

  const store_engine::StoreEngine &SoftadastraRuntime::store() const noexcept
  {
    return store_;
  }

  sync_engine::SyncEngine &SoftadastraRuntime::sync() noexcept
  {
    return sync_;
  }

  const sync_engine::SyncEngine &SoftadastraRuntime::sync() const noexcept
  {
    return sync_;
  }

  transport_engine::TransportEngine &SoftadastraRuntime::transport() noexcept
  {
    return transport_;
  }

  const transport_engine::TransportEngine &SoftadastraRuntime::transport() const noexcept
  {
    return transport_;
  }

  discovery_service::DiscoveryService &SoftadastraRuntime::discovery() noexcept
  {
    return discovery_;
  }

  const discovery_service::DiscoveryService &SoftadastraRuntime::discovery() const noexcept
  {
    return discovery_;
  }

  metadata_service::MetadataService &SoftadastraRuntime::metadata() noexcept
  {
    return metadata_;
  }

  const metadata_service::MetadataService &SoftadastraRuntime::metadata() const noexcept
  {
    return metadata_;
  }

  store_core::StoreConfig SoftadastraRuntime::make_store_config()
  {
    fs::create_directories("data");

    store_core::StoreConfig config;
    config.wal_path = "data/wal.log";
    config.enable_wal = true;
    config.initial_capacity = 1024;
    config.auto_flush = true;

    return config;
  }

  sync_core::SyncConfig SoftadastraRuntime::make_sync_config()
  {
    sync_core::SyncConfig config;

    config.node_id = "node-1";
    config.batch_size = 64;
    config.max_retries = 5;
    config.retry_interval_ms = 5000;
    config.ack_timeout_ms = 10000;
    config.auto_queue = true;
    config.require_ack = true;

    return config;
  }

  transport_core::TransportConfig SoftadastraRuntime::make_transport_config()
  {
    transport_core::TransportConfig config;

    config.bind_host = "0.0.0.0";
    config.bind_port = 9500;
    config.connect_timeout_ms = 5000;
    config.read_timeout_ms = 5000;
    config.write_timeout_ms = 5000;
    config.max_frame_size = 1024 * 1024;
    config.max_pending_messages = 1024;
    config.enable_keepalive = true;
    config.keepalive_interval_ms = 10000;

    return config;
  }

  discovery_service::DiscoveryOptions
  SoftadastraRuntime::make_discovery_options(const std::string &node_id)
  {
    discovery_service::DiscoveryOptions options;

    options.node_id = node_id;
    options.bind_host = "0.0.0.0";
    options.bind_port = 9400;
    options.broadcast_host = "255.255.255.255";
    options.broadcast_port = 9400;
    options.announce_host = "127.0.0.1";
    options.announce_port = 9500;
    options.announce_interval_ms = 3000;
    options.peer_ttl_ms = 15000;
    options.enable_broadcast = true;

    return options;
  }

  metadata_service::MetadataOptions
  SoftadastraRuntime::make_metadata_options(const std::string &node_id,
                                            const std::string &version)
  {
    metadata_service::MetadataOptions options;

    options.node_id = node_id;
    options.display_name = "Softadastra Node";
    options.version = version.empty() ? "0.1.0" : version;
    options.auto_refresh = true;
    options.refresh_interval_ms = 5000;

    return options;
  }

  void SoftadastraRuntime::register_commands()
  {
    namespace cli_command = softadastra::cli::command;
    namespace cli_types = softadastra::cli::types;

    namespace node_commands = softadastra::app::cli::commands::node;
    namespace peers_commands = softadastra::app::cli::commands::peers;
    namespace status_commands = softadastra::app::cli::commands::status;
    namespace store_commands = softadastra::app::cli::commands::store;
    namespace sync_commands = softadastra::app::cli::commands::sync;

    cli_.register_command(
        cli_command::CliCommand{
            "status",
            "Show Softadastra runtime status",
            "status",
            cli_types::CliCommandType::Info,
            {},
            {}},
        std::make_shared<status_commands::StatusCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "node-info",
            "Show local node information",
            "node-info",
            cli_types::CliCommandType::Info,
            {"node"},
            {}},
        std::make_shared<node_commands::NodeInfoCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "node-start",
            "Start local Softadastra node services",
            "node-start",
            cli_types::CliCommandType::System,
            {"start"},
            {}},
        std::make_shared<node_commands::NodeStartCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "store-put",
            "Write a key/value pair into the local store",
            "store-put <key> <value>",
            cli_types::CliCommandType::Custom,
            {"put"},
            {}},
        std::make_shared<store_commands::StorePutCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "store-get",
            "Read one key from the local store",
            "store-get <key>",
            cli_types::CliCommandType::Custom,
            {"get"},
            {}},
        std::make_shared<store_commands::StoreGetCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "sync-status",
            "Show current sync engine state",
            "sync-status",
            cli_types::CliCommandType::Info,
            {},
            {}},
        std::make_shared<sync_commands::SyncStatusCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "sync-tick",
            "Run one manual sync scheduler cycle",
            "sync-tick",
            cli_types::CliCommandType::System,
            {},
            {}},
        std::make_shared<sync_commands::SyncTickCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "peers",
            "List discovery and transport peers",
            "peers",
            cli_types::CliCommandType::Info,
            {},
            {}},
        std::make_shared<peers_commands::PeersCommand>(*this));
  }
} // namespace softadastra::app::cli
