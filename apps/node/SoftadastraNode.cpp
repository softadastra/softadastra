/*
 * SoftadastraNode.cpp
 */

#include "SoftadastraNode.hpp"

#include <chrono>
#include <filesystem>
#include <thread>

namespace softadastra::app::node
{
  namespace fs = std::filesystem;

  SoftadastraNode::SoftadastraNode()
      : store_config_(make_store_config()),
        store_(store_config_),
        sync_config_(make_sync_config()),
        sync_context_{},
        sync_([&]() -> sync_core::SyncContext &
              {
                sync_context_.store = &store_;
                sync_context_.config = &sync_config_;
                return sync_context_; }()),
        scheduler_(sync_),
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
        metadata_options_(make_metadata_options(sync_config_.node_id)),
        metadata_(metadata_options_, discovery_.engine()),
        valid_(false),
        running_(false)
  {
    valid_ =
        sync_context_.valid() &&
        transport_context_.valid() &&
        discovery_options_.valid() &&
        metadata_options_.valid();
  }

  SoftadastraNode::~SoftadastraNode()
  {
    stop();
  }

  bool SoftadastraNode::valid() const noexcept
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

  bool SoftadastraNode::running() const noexcept
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

  const std::string &SoftadastraNode::node_id() const
  {
    return sync_config_.node_id;
  }

  store_core::StoreConfig SoftadastraNode::make_store_config()
  {
    fs::create_directories("data");

    store_core::StoreConfig config;
    config.wal_path = "data/wal.log";
    config.enable_wal = true;
    config.initial_capacity = 1024;
    config.auto_flush = true;

    return config;
  }

  sync_core::SyncConfig SoftadastraNode::make_sync_config()
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

  transport_core::TransportConfig SoftadastraNode::make_transport_config()
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
  SoftadastraNode::make_discovery_options(const std::string &node_id)
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
  SoftadastraNode::make_metadata_options(const std::string &node_id)
  {
    metadata_service::MetadataOptions options;

    options.node_id = node_id;
    options.display_name = "Softadastra Node";
    options.version = "0.1.0";
    options.auto_refresh = true;
    options.refresh_interval_ms = 5000;

    return options;
  }

  void SoftadastraNode::tick_discovery()
  {
    if (!discovery_.running())
    {
      return;
    }

    discovery_.poll_many(32);
  }

  void SoftadastraNode::tick_transport()
  {
    if (!transport_.running())
    {
      return;
    }

    transport_.poll_many(32);
  }

  void SoftadastraNode::tick_sync()
  {
    const auto tick_result = scheduler_.tick(false);

    if (tick_result.batch.empty())
    {
      return;
    }

    const auto peers = transport_.peers().connected_peers();

    for (const auto &peer_session : peers)
    {
      if (!peer_session.valid() || !peer_session.connected())
      {
        continue;
      }

      transport_.send_sync_batch(peer_session.peer, tick_result.batch);
    }
  }

} // namespace softadastra::app::node
