/**
 *
 *  @file SoftadastraRuntime.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Softadastra.
 *  All rights reserved.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *
 *  Softadastra CLI App
 *
 */

#ifndef SOFTADASTRA_APPS_CLI_SOFTADASTRA_RUNTIME_HPP
#define SOFTADASTRA_APPS_CLI_SOFTADASTRA_RUNTIME_HPP

#include <string>

#include <softadastra/cli/cli.hpp>

#include <softadastra/discovery/Discovery.hpp>
#include <softadastra/metadata/Metadata.hpp>
#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>
#include <softadastra/transport/Transport.hpp>

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
   * @brief Composes the runtime used by the Softadastra CLI application.
   *
   * SoftadastraRuntime owns and wires the real Softadastra modules used by the
   * app-level CLI.
   *
   * It owns:
   * - store engine
   * - sync engine
   * - transport backend and engine
   * - discovery service
   * - metadata service
   * - CLI service
   *
   * The runtime is responsible for dependency wiring only. It does not
   * reimplement business logic from lower-level modules.
   *
   * Commands receive a reference to this runtime and call the relevant module
   * APIs through explicit accessors.
   */
  class SoftadastraRuntime
  {
  public:
    /**
     * @brief Builds a Softadastra runtime from CLI configuration.
     *
     * @param cli_config CLI configuration.
     */
    explicit SoftadastraRuntime(const cli_core::CliConfig &cli_config);

    /**
     * @brief Copy construction is disabled.
     */
    SoftadastraRuntime(const SoftadastraRuntime &) = delete;

    /**
     * @brief Copy assignment is disabled.
     */
    SoftadastraRuntime &operator=(const SoftadastraRuntime &) = delete;

    /**
     * @brief Move construction is disabled.
     */
    SoftadastraRuntime(SoftadastraRuntime &&) = delete;

    /**
     * @brief Move assignment is disabled.
     */
    SoftadastraRuntime &operator=(SoftadastraRuntime &&) = delete;

    /**
     * @brief Stops node services before destroying the runtime.
     */
    ~SoftadastraRuntime();

    /**
     * @brief Returns true if the runtime was initialized correctly.
     *
     * @return true when all composed modules are usable.
     */
    [[nodiscard]] bool is_valid() const noexcept;

    /**
     * @brief Backward-compatible alias for is_valid().
     *
     * @return true when the runtime is valid.
     */
    [[nodiscard]] bool valid() const noexcept
    {
      return is_valid();
    }

    /**
     * @brief Starts node-level services.
     *
     * Services are started in dependency order:
     * - transport
     * - discovery
     * - metadata
     *
     * @return true on success.
     */
    [[nodiscard]] bool start_node();

    /**
     * @brief Stops node-level services.
     *
     * Services are stopped in reverse operational order.
     */
    void stop_node();

    /**
     * @brief Returns true if node-level services are considered running.
     *
     * @return true when node services are running.
     */
    [[nodiscard]] bool node_running() const noexcept;

    /**
     * @brief Returns the configured local node id.
     *
     * @return Node id.
     */
    [[nodiscard]] const std::string &node_id() const noexcept;

    /**
     * @brief Returns the CLI service.
     *
     * @return CLI service.
     */
    [[nodiscard]] cli_service::CliService &cli() noexcept;

    /**
     * @brief Returns the CLI service.
     *
     * @return CLI service.
     */
    [[nodiscard]] const cli_service::CliService &cli() const noexcept;

    /**
     * @brief Returns the local store engine.
     *
     * @return Store engine.
     */
    [[nodiscard]] store_engine::StoreEngine &store() noexcept;

    /**
     * @brief Returns the local store engine.
     *
     * @return Store engine.
     */
    [[nodiscard]] const store_engine::StoreEngine &store() const noexcept;

    /**
     * @brief Returns the local sync engine.
     *
     * @return Sync engine.
     */
    [[nodiscard]] sync_engine::SyncEngine &sync() noexcept;

    /**
     * @brief Returns the local sync engine.
     *
     * @return Sync engine.
     */
    [[nodiscard]] const sync_engine::SyncEngine &sync() const noexcept;

    /**
     * @brief Returns the transport engine.
     *
     * @return Transport engine.
     */
    [[nodiscard]] transport_engine::TransportEngine &transport() noexcept;

    /**
     * @brief Returns the transport engine.
     *
     * @return Transport engine.
     */
    [[nodiscard]] const transport_engine::TransportEngine &transport() const noexcept;

    /**
     * @brief Returns the discovery service.
     *
     * @return Discovery service.
     */
    [[nodiscard]] discovery_service::DiscoveryService &discovery() noexcept;

    /**
     * @brief Returns the discovery service.
     *
     * @return Discovery service.
     */
    [[nodiscard]] const discovery_service::DiscoveryService &discovery() const noexcept;

    /**
     * @brief Returns the metadata service.
     *
     * @return Metadata service.
     */
    [[nodiscard]] metadata_service::MetadataService &metadata() noexcept;

    /**
     * @brief Returns the metadata service.
     *
     * @return Metadata service.
     */
    [[nodiscard]] const metadata_service::MetadataService &metadata() const noexcept;

  private:
    /**
     * @brief Creates the store configuration used by the CLI runtime.
     *
     * @return Store configuration.
     */
    [[nodiscard]] static store_core::StoreConfig make_store_config();

    /**
     * @brief Creates the sync configuration for a node id.
     *
     * @param node_id Local node id.
     * @return Sync configuration.
     */
    [[nodiscard]] static sync_core::SyncConfig make_sync_config(
        const std::string &node_id);

    /**
     * @brief Creates the transport configuration used by the CLI runtime.
     *
     * @return Transport configuration.
     */
    [[nodiscard]] static transport_core::TransportConfig make_transport_config();

    /**
     * @brief Creates discovery service options.
     *
     * @param node_id Local node id.
     * @return Discovery options.
     */
    [[nodiscard]] static discovery_service::DiscoveryOptions make_discovery_options(
        const std::string &node_id);

    /**
     * @brief Creates metadata service options.
     *
     * @param node_id Local node id.
     * @param version Runtime version.
     * @return Metadata options.
     */
    [[nodiscard]] static metadata_service::MetadataOptions make_metadata_options(
        const std::string &node_id,
        const std::string &version);

    /**
     * @brief Registers all app-level commands into the CLI service.
     */
    void register_commands();

  private:
    cli_core::CliConfig cli_config_{};

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

#endif // SOFTADASTRA_APPS_CLI_SOFTADASTRA_RUNTIME_HPP
