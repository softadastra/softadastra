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

#include <filesystem>
#include <memory>
#include <utility>

namespace softadastra::app::cli
{
  namespace fs = std::filesystem;

  namespace core_time = softadastra::core::time;

  namespace cli_command = softadastra::cli::command;
  namespace cli_types = softadastra::cli::types;

  namespace node_commands = softadastra::app::cli::commands::node;
  namespace peers_commands = softadastra::app::cli::commands::peers;
  namespace status_commands = softadastra::app::cli::commands::status;
  namespace store_commands = softadastra::app::cli::commands::store;
  namespace sync_commands = softadastra::app::cli::commands::sync;

  namespace
  {
    constexpr const char *DEFAULT_NODE_ID = "node-1";
    constexpr const char *DEFAULT_VERSION = "0.1.0";
    constexpr const char *DEFAULT_DISPLAY_NAME = "Softadastra Node";

    constexpr const char *DATA_DIRECTORY = "data";
    constexpr const char *STORE_WAL_PATH = "data/wal.log";

    constexpr std::uint16_t DISCOVERY_PORT = 9400;
    constexpr std::uint16_t TRANSPORT_PORT = 9500;

    constexpr std::size_t SYNC_BATCH_SIZE = 64;
    constexpr std::uint32_t SYNC_MAX_RETRIES = 5;

    constexpr std::size_t TRANSPORT_MAX_FRAME_SIZE = 1024 * 1024;
    constexpr std::size_t TRANSPORT_MAX_PENDING_MESSAGES = 1024;
  }

  SoftadastraRuntime::SoftadastraRuntime(
      const cli_core::CliConfig &cli_config)
      : cli_config_(cli_config),
        store_config_(make_store_config()),
        store_(store_config_),
        sync_config_(make_sync_config(DEFAULT_NODE_ID)),
        sync_context_(store_, sync_config_),
        sync_(sync_context_),
        transport_config_(make_transport_config()),
        transport_context_(transport_config_, sync_),
        transport_backend_(transport_config_),
        transport_(transport_context_, transport_backend_),
        discovery_options_(make_discovery_options(sync_config_.node_id)),
        discovery_(discovery_options_, transport_),
        metadata_options_(make_metadata_options(
            sync_config_.node_id,
            cli_config_.version)),
        metadata_(metadata_options_, discovery_.engine()),
        cli_(cli_config_),
        valid_(false),
        node_running_(false)
  {
    valid_ =
        cli_config_.valid() &&
        sync_context_.is_valid() &&
        transport_config_.is_valid() &&
        discovery_options_.is_valid() &&
        metadata_options_.is_valid();

    if (valid_)
    {
      register_commands();
    }
  }

  SoftadastraRuntime::~SoftadastraRuntime()
  {
    stop_node();
  }

  bool SoftadastraRuntime::is_valid() const noexcept
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

  const std::string &SoftadastraRuntime::node_id() const noexcept
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

  const discovery_service::DiscoveryService &
  SoftadastraRuntime::discovery() const noexcept
  {
    return discovery_;
  }

  metadata_service::MetadataService &SoftadastraRuntime::metadata() noexcept
  {
    return metadata_;
  }

  const metadata_service::MetadataService &
  SoftadastraRuntime::metadata() const noexcept
  {
    return metadata_;
  }

  store_core::StoreConfig SoftadastraRuntime::make_store_config()
  {
    fs::create_directories(DATA_DIRECTORY);

    return store_core::StoreConfig::durable(STORE_WAL_PATH);
  }

  sync_core::SyncConfig SoftadastraRuntime::make_sync_config(
      const std::string &node_id)
  {
    auto config =
        sync_core::SyncConfig::durable(node_id);

    config.batch_size = SYNC_BATCH_SIZE;
    config.max_retries = SYNC_MAX_RETRIES;
    config.retry_interval = core_time::Duration::from_seconds(5);
    config.ack_timeout = core_time::Duration::from_seconds(10);
    config.auto_queue = true;
    config.require_ack = true;

    return config;
  }

  transport_core::TransportConfig
  SoftadastraRuntime::make_transport_config()
  {
    auto config =
        transport_core::TransportConfig::local(TRANSPORT_PORT);

    config.bind_host = "0.0.0.0";
    config.connect_timeout = core_time::Duration::from_seconds(5);
    config.read_timeout = core_time::Duration::from_seconds(5);
    config.write_timeout = core_time::Duration::from_seconds(5);
    config.max_frame_size = TRANSPORT_MAX_FRAME_SIZE;
    config.max_pending_messages = TRANSPORT_MAX_PENDING_MESSAGES;
    config.enable_keepalive = true;
    config.keepalive_interval = core_time::Duration::from_seconds(10);

    return config;
  }

  discovery_service::DiscoveryOptions
  SoftadastraRuntime::make_discovery_options(
      const std::string &node_id)
  {
    auto options =
        discovery_service::DiscoveryOptions::lan(
            node_id,
            DISCOVERY_PORT,
            TRANSPORT_PORT);

    options.announce_host = "127.0.0.1";
    options.announce_interval = core_time::Duration::from_seconds(3);
    options.peer_ttl = core_time::Duration::from_seconds(15);
    options.enable_broadcast = true;

    return options;
  }

  metadata_service::MetadataOptions
  SoftadastraRuntime::make_metadata_options(
      const std::string &node_id,
      const std::string &version)
  {
    auto options =
        metadata_service::MetadataOptions::local(
            node_id,
            version.empty() ? DEFAULT_VERSION : version);

    options.display_name = DEFAULT_DISPLAY_NAME;
    options.auto_refresh = true;
    options.refresh_interval = core_time::Duration::from_seconds(5);

    return options;
  }

  void SoftadastraRuntime::register_commands()
  {
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
            cli_types::CliCommandType::Admin,
            {"start"},
            {}},
        std::make_shared<node_commands::NodeStartCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "store-put",
            "Write a key/value pair into the local store",
            "store-put <key> <value>",
            cli_types::CliCommandType::Admin,
            {"put"},
            {
                {
                    "help",
                    "h",
                    "Show help for this command",
                    "",
                    false,
                    false,
                },
            }},
        std::make_shared<store_commands::StorePutCommand>(*this));

    cli_.register_command(
        cli_command::CliCommand{
            "store-get",
            "Read one key from the local store",
            "store-get <key>",
            cli_types::CliCommandType::Info,
            {"get"},
            {
                {
                    "help",
                    "h",
                    "Show help for this command",
                    "",
                    false,
                    false,
                },
            }},
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
            cli_types::CliCommandType::Admin,
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
