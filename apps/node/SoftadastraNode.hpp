/*
 * SoftadastraNode.hpp
 */

#ifndef SOFTADASTRA_APPS_NODE_SOFTADASTRA_NODE_HPP
#define SOFTADASTRA_APPS_NODE_SOFTADASTRA_NODE_HPP

#include <chrono>
#include <cstdint>
#include <string>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>

#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>
#include <softadastra/sync/scheduler/SyncScheduler.hpp>

#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/DiscoveryService.hpp>

#include <softadastra/metadata/MetadataOptions.hpp>
#include <softadastra/metadata/MetadataService.hpp>

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
   * @brief Long-running Softadastra node daemon
   *
   * Owns and drives the runtime modules:
   * - store
   * - sync
   * - transport
   * - discovery
   * - metadata
   */
  class SoftadastraNode
  {
  public:
    SoftadastraNode();

    SoftadastraNode(const SoftadastraNode &) = delete;
    SoftadastraNode &operator=(const SoftadastraNode &) = delete;

    SoftadastraNode(SoftadastraNode &&) = delete;
    SoftadastraNode &operator=(SoftadastraNode &&) = delete;

    /**
     * @brief Stop services on destruction
     */
    ~SoftadastraNode();

    /**
     * @brief Return true if the node was initialized correctly
     */
    bool valid() const noexcept;

    /**
     * @brief Start node services
     */
    bool start();

    /**
     * @brief Stop node services
     */
    void stop();

    /**
     * @brief Return true if node services are running
     */
    bool running() const noexcept;

    /**
     * @brief Execute one deterministic node cycle
     */
    void tick();

    /**
     * @brief Return local node id
     */
    const std::string &node_id() const;

  private:
    static store_core::StoreConfig make_store_config();
    static sync_core::SyncConfig make_sync_config();
    static transport_core::TransportConfig make_transport_config();
    static discovery_service::DiscoveryOptions make_discovery_options(
        const std::string &node_id);
    static metadata_service::MetadataOptions make_metadata_options(
        const std::string &node_id);

    void tick_discovery();
    void tick_transport();
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

#endif
