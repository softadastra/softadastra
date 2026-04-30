/*
 * SoftadastraNode.cpp
 */

#include "SoftadastraNode.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <thread>

namespace softadastra::app::node
{
  namespace fs = std::filesystem;
  namespace core_time = softadastra::core::time;

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

    constexpr std::size_t DISCOVERY_POLL_LIMIT = 32;
    constexpr std::size_t TRANSPORT_POLL_LIMIT = 32;
  }

  SoftadastraNode::SoftadastraNode()
      : store_config_(make_store_config()),
        store_(store_config_),
        sync_config_(make_sync_config(DEFAULT_NODE_ID)),
        sync_context_(store_, sync_config_),
        sync_(sync_context_),
        scheduler_(sync_),
        transport_config_(make_transport_config()),
        transport_context_(transport_config_, sync_),
        transport_backend_(transport_config_),
        transport_(transport_context_, transport_backend_),
        discovery_options_(make_discovery_options(sync_config_.node_id)),
        discovery_(discovery_options_, transport_),
        metadata_options_(make_metadata_options(sync_config_.node_id)),
        metadata_(metadata_options_, discovery_.engine()),
        valid_(false),
        running_(false)
  {
    valid_ =
        sync_context_.is_valid() &&
        transport_config_.is_valid() &&
        discovery_options_.is_valid() &&
        metadata_options_.is_valid();
  }

  SoftadastraNode::~SoftadastraNode()
  {
    stop();
  }

  bool SoftadastraNode::is_valid() const noexcept
  {
    return valid_;
  }

  bool SoftadastraNode::start()
  {
    if (!valid_)
    {
      return false;
    }

    if (running_)
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

    running_ = true;
    return true;
  }

  void SoftadastraNode::stop()
  {
    if (!running_)
    {
      return;
    }

    metadata_.stop();
    discovery_.stop();
    transport_.stop();

    running_ = false;
  }

  bool SoftadastraNode::is_running() const noexcept
  {
    return running_;
  }

  void SoftadastraNode::tick()
  {
    if (!running_)
    {
      return;
    }

    tick_discovery();
    tick_transport();
    tick_sync();

    std::this_thread::sleep_for(tick_interval_);
  }

  const std::string &SoftadastraNode::node_id() const noexcept
  {
    return sync_config_.node_id;
  }

  store_core::StoreConfig SoftadastraNode::make_store_config()
  {
    fs::create_directories(DATA_DIRECTORY);

    return store_core::StoreConfig::durable(STORE_WAL_PATH);
  }

  sync_core::SyncConfig SoftadastraNode::make_sync_config(
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
  SoftadastraNode::make_transport_config()
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
  SoftadastraNode::make_discovery_options(
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
  SoftadastraNode::make_metadata_options(
      const std::string &node_id)
  {
    auto options =
        metadata_service::MetadataOptions::local(
            node_id,
            DEFAULT_VERSION);

    options.display_name = DEFAULT_DISPLAY_NAME;
    options.auto_refresh = true;
    options.refresh_interval = core_time::Duration::from_seconds(5);

    return options;
  }

  void SoftadastraNode::tick_discovery()
  {
    if (!discovery_.is_running())
    {
      return;
    }

    discovery_.poll_many(DISCOVERY_POLL_LIMIT);
  }

  void SoftadastraNode::tick_transport()
  {
    if (!transport_.is_running())
    {
      return;
    }

    transport_.poll_many(TRANSPORT_POLL_LIMIT);
  }

  void SoftadastraNode::tick_sync()
  {
    const auto tick_result =
        scheduler_.tick(false);

    if (tick_result.batch.empty())
    {
      return;
    }

    const auto peers =
        transport_.peers().connected_peers();

    for (const auto &peer_session : peers)
    {
      if (!peer_session.is_valid() ||
          !peer_session.connected())
      {
        continue;
      }

      (void)transport_.send_sync_batch(
          peer_session.peer,
          tick_result.batch);
    }
  }

} // namespace softadastra::app::node
