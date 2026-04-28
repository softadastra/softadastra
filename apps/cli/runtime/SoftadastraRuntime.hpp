/*
 * SoftadastraRuntime.hpp
 */

#ifndef SOFTADASTRA_APPS_CLI_SOFTADASTRA_RUNTIME_HPP
#define SOFTADASTRA_APPS_CLI_SOFTADASTRA_RUNTIME_HPP

#include <cstdint>
#include <string>

#include <softadastra/cli/cli.hpp>

#include <softadastra/store/core/StoreConfig.hpp>
#include <softadastra/store/engine/StoreEngine.hpp>

#include <softadastra/sync/core/SyncConfig.hpp>
#include <softadastra/sync/core/SyncContext.hpp>
#include <softadastra/sync/engine/SyncEngine.hpp>

#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportContext.hpp>
#include <softadastra/transport/engine/TransportEngine.hpp>

#include <softadastra/discovery/DiscoveryOptions.hpp>
#include <softadastra/discovery/DiscoveryService.hpp>

#include <softadastra/metadata/MetadataOptions.hpp>
#include <softadastra/metadata/MetadataService.hpp>

namespace softadastra::app::cli
{
  namespace cli_core = softadastra::cli::core;
  namespace cli_service = softadastra::cli;

  namespace store_core = softadastra::store::core;
  namespace store_engine = softadastra::store::engine;

  namespace sync_core = softadastra::sync::core;
  namespace sync_engine = softadastra::sync::engine;

  namespace transport_backend = softadastra::transport::backend;
  namespace transport_core = softadastra::transport::core;
  namespace transport_engine = softadastra::transport::engine;

  namespace discovery_service = softadastra::discovery;

  namespace metadata_service = softadastra::metadata;

  /**
   * @brief Runtime composition layer for the Softadastra CLI application
   *
   * This class owns and wires the real Softadastra runtime modules:
   * - store
   * - sync
   * - transport
   * - discovery
   * - metadata
   * - cli
   *
   * apps/cli must not reimplement engine logic.
   * It only composes modules and exposes them to CLI commands.
   */
  class SoftadastraRuntime
  {
  public:
    /**
     * @brief Build a Softadastra runtime from CLI configuration
     */
    explicit SoftadastraRuntime(const cli_core::CliConfig &cli_config);

    SoftadastraRuntime(const SoftadastraRuntime &) = delete;
    SoftadastraRuntime &operator=(const SoftadastraRuntime &) = delete;

    SoftadastraRuntime(SoftadastraRuntime &&) = delete;
    SoftadastraRuntime &operator=(SoftadastraRuntime &&) = delete;

    /**
     * @brief Stop runtime services if needed
     */
    ~SoftadastraRuntime();

    /**
     * @brief Return true if the runtime was initialized correctly
     */
    bool valid() const noexcept;

    /**
     * @brief Start node-level services
     *
     * Starts sync-facing runtime services:
     * - transport
     * - discovery
     * - metadata
     */
    bool start_node();

    /**
     * @brief Stop node-level services
     */
    void stop_node();

    /**
     * @brief Return true if node-level services are running
     */
    bool node_running() const noexcept;

    /**
     * @brief Return configured local node id
     */
    const std::string &node_id() const;

    /**
     * @brief Access CLI service
     */
    cli_service::CliService &cli() noexcept;

    /**
     * @brief Access CLI service
     */
    const cli_service::CliService &cli() const noexcept;

    /**
     * @brief Access store engine
     */
    store_engine::StoreEngine &store() noexcept;

    /**
     * @brief Access store engine
     */
    const store_engine::StoreEngine &store() const noexcept;

    /**
     * @brief Access sync engine
     */
    sync_engine::SyncEngine &sync() noexcept;

    /**
     * @brief Access sync engine
     */
    const sync_engine::SyncEngine &sync() const noexcept;

    /**
     * @brief Access transport engine
     */
    transport_engine::TransportEngine &transport() noexcept;

    /**
     * @brief Access transport engine
     */
    const transport_engine::TransportEngine &transport() const noexcept;

    /**
     * @brief Access discovery service
     */
    discovery_service::DiscoveryService &discovery() noexcept;

    /**
     * @brief Access discovery service
     */
    const discovery_service::DiscoveryService &discovery() const noexcept;

    /**
     * @brief Access metadata service
     */
    metadata_service::MetadataService &metadata() noexcept;

    /**
     * @brief Access metadata service
     */
    const metadata_service::MetadataService &metadata() const noexcept;

  private:
    static store_core::StoreConfig make_store_config();
    static sync_core::SyncConfig make_sync_config();
    static transport_core::TransportConfig make_transport_config();
    static discovery_service::DiscoveryOptions make_discovery_options(
        const std::string &node_id);
    static metadata_service::MetadataOptions make_metadata_options(
        const std::string &node_id,
        const std::string &version);

    void register_commands();

  private:
    cli_core::CliConfig cli_config_;

    store_core::StoreConfig store_config_;
    store_engine::StoreEngine store_;

    sync_core::SyncConfig sync_config_;
    sync_core::SyncContext sync_context_;
    sync_engine::SyncEngine sync_;

    transport_core::TransportConfig transport_config_;
    transport_core::TransportContext transport_context_;
    transport_backend::TcpTransportBackend transport_backend_;
    transport_engine::TransportEngine transport_;

    discovery_service::DiscoveryOptions discovery_options_;
    discovery_service::DiscoveryService discovery_;

    metadata_service::MetadataOptions metadata_options_;
    metadata_service::MetadataService metadata_;

    cli_service::CliService cli_;

    bool valid_{false};
    bool node_running_{false};
  };

} // namespace softadastra::app::cli

#endif
