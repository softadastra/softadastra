/**
 *
 *  @file SoftadastraNode.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra Node App
 *
 */

#ifndef SOFTADASTRA_APPS_NODE_SOFTADASTRA_NODE_HPP
#define SOFTADASTRA_APPS_NODE_SOFTADASTRA_NODE_HPP

#include <chrono>
#include <string>

#include <softadastra/discovery/Discovery.hpp>
#include <softadastra/metadata/Metadata.hpp>
#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

namespace softadastra::app::node
{
  namespace store_core = softadastra::store::core;
  namespace store_engine = softadastra::store::engine;

  namespace sync_core = softadastra::sync::core;
  namespace sync_engine = softadastra::sync::engine;
  namespace sync_scheduler = softadastra::sync::scheduler;

  namespace transport_backend = softadastra::transport::backend;
  namespace transport_core = softadastra::transport::core;
  namespace transport_engine = softadastra::transport::engine;

  namespace discovery_service = softadastra::discovery;
  namespace metadata_service = softadastra::metadata;

  /**
   * @brief Long-running Softadastra node daemon.
   *
   * SoftadastraNode owns and drives the runtime modules required by a local
   * Softadastra node.
   *
   * It owns:
   * - store engine
   * - sync engine
   * - sync scheduler
   * - transport backend and engine
   * - discovery service
   * - metadata service
   *
   * The node daemon is responsible for lifecycle orchestration and deterministic
   * ticking. It does not reimplement module business logic.
   */
  class SoftadastraNode
  {
  public:
    /**
     * @brief Creates a Softadastra node daemon.
     */
    SoftadastraNode();

    /**
     * @brief Copy construction is disabled.
     */
    SoftadastraNode(const SoftadastraNode &) = delete;

    /**
     * @brief Copy assignment is disabled.
     */
    SoftadastraNode &operator=(const SoftadastraNode &) = delete;

    /**
     * @brief Move construction is disabled.
     */
    SoftadastraNode(SoftadastraNode &&) = delete;

    /**
     * @brief Move assignment is disabled.
     */
    SoftadastraNode &operator=(SoftadastraNode &&) = delete;

    /**
     * @brief Stops services before destroying the node daemon.
     */
    ~SoftadastraNode();

    /**
     * @brief Returns true if the node was initialized correctly.
     *
     * @return true when all composed modules are usable.
     */
    [[nodiscard]] bool is_valid() const noexcept;

    /**
     * @brief Backward-compatible alias for is_valid().
     *
     * @return true when the node is valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Starts node services.
     *
     * Services are started in dependency order:
     * - transport
     * - discovery
     * - metadata
     *
     * @return true on success.
     */
    [[nodiscard]] bool start();

    /**
     * @brief Stops node services.
     *
     * Services are stopped in reverse operational order.
     */
    void stop();

    /**
     * @brief Returns true if node services are running.
     *
     * @return true when running.
     */
    [[nodiscard]] bool is_running() const noexcept;

    /**
     * @brief Backward-compatible alias for is_running().
     *
     * @return true when running.
     */
    [[nodiscard]] bool running() const noexcept
    {
      return is_running();
    }

    /**
     * @brief Executes one deterministic node cycle.
     *
     * A cycle may poll discovery, poll transport, retry sync operations, and
     * send ready sync batches to connected peers.
     */
    void tick();

    /**
     * @brief Returns the local node id.
     *
     * @return Node id.
     */
    [[nodiscard]] const std::string &node_id() const noexcept;

    /**
     * @brief Returns the configured tick interval.
     *
     * @return Tick interval.
     */
    [[nodiscard]] std::chrono::milliseconds tick_interval() const noexcept
    {
      return tick_interval_;
    }

    /**
     * @brief Updates the daemon tick interval.
     *
     * Non-positive intervals are ignored.
     *
     * @param interval New tick interval.
     */
    void set_tick_interval(std::chrono::milliseconds interval) noexcept
    {
      if (interval.count() > 0)
      {
        tick_interval_ = interval;
      }
    }

  private:
    /**
     * @brief Creates the store configuration used by the node.
     *
     * @return Store configuration.
     */
    [[nodiscard]] static store_core::StoreConfig make_store_config();

    /**
     * @brief Creates sync configuration for a node id.
     *
     * @param node_id Local node id.
     * @return Sync configuration.
     */
    [[nodiscard]] static sync_core::SyncConfig make_sync_config(
        const std::string &node_id);

    /**
     * @brief Creates transport configuration.
     *
     * @return Transport configuration.
     */
    [[nodiscard]] static transport_core::TransportConfig make_transport_config();

    /**
     * @brief Creates discovery options.
     *
     * @param node_id Local node id.
     * @return Discovery options.
     */
    [[nodiscard]] static discovery_service::DiscoveryOptions make_discovery_options(
        const std::string &node_id);

    /**
     * @brief Creates metadata options.
     *
     * @param node_id Local node id.
     * @return Metadata options.
     */
    [[nodiscard]] static metadata_service::MetadataOptions make_metadata_options(
        const std::string &node_id);

    /**
     * @brief Advances discovery state once.
     */
    void tick_discovery();

    /**
     * @brief Advances transport state once.
     */
    void tick_transport();

    /**
     * @brief Advances sync state once.
     */
    void tick_sync();

  private:
    store_core::StoreConfig store_config_;
    store_engine::StoreEngine store_;

    sync_core::SyncConfig sync_config_;
    sync_core::SyncContext sync_context_;
    sync_engine::SyncEngine sync_;
    sync_scheduler::SyncScheduler scheduler_;

    transport_core::TransportConfig transport_config_;
    transport_core::TransportContext transport_context_;
    transport_backend::TcpTransportBackend transport_backend_;
    transport_engine::TransportEngine transport_;

    discovery_service::DiscoveryOptions discovery_options_;
    discovery_service::DiscoveryService discovery_;

    metadata_service::MetadataOptions metadata_options_;
    metadata_service::MetadataService metadata_;

    bool valid_{false};
    bool running_{false};

    std::chrono::milliseconds tick_interval_{10};
  };

} // namespace softadastra::app::node

#endif // SOFTADASTRA_APPS_NODE_SOFTADASTRA_NODE_HPP
