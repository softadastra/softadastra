/**
 *
 *  @file Cli.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#ifndef SOFTADASTRA_CLI_CLI_HPP
#define SOFTADASTRA_CLI_CLI_HPP

#include "control/ControlClient.hpp"
#include "platform/ManagedNetwork.hpp"
#include "platform/Network.hpp"

namespace softadastra
{
  /**
   * @brief Provides the Softadastra command-line interface.
   *
   * Cli translates command-line arguments into Host control operations through
   * ControlClient. Optional local network providers allow read-only network
   * inspection and managed-network operations without routing those operations
   * through the Host control interface.
   */
  class Cli
  {
  public:
    /**
     * @brief Creates a command-line interface backed by a Host control client.
     *
     * @param client Control client used to communicate with the Host.
     */
    explicit Cli(
        ControlClient &client) noexcept;

    /**
     * @brief Creates a CLI with a local read-only network observer.
     *
     * @param client Control client used to communicate with the Host.
     * @param network Local network provider used for network inspection.
     */
    Cli(
        ControlClient &client,
        const Network &network) noexcept;

    /**
     * @brief Creates a CLI with local network and managed-network providers.
     *
     * @param client Control client used to communicate with the Host.
     * @param network Local network provider used for network inspection.
     * @param managed_network Managed network provider used for local network
     *        lifecycle operations.
     */
    Cli(
        ControlClient &client,
        const Network &network,
        ManagedNetwork &managed_network) noexcept;

    /**
     * @brief Executes a Softadastra command-line operation.
     *
     * @param argc Number of command-line arguments.
     * @param argv Command-line arguments.
     *
     * @return 0 on success, 1 when an operation fails, or 2 when command usage
     *         is invalid.
     */
    int run(
        int argc,
        const char *const argv[]);

  private:
    ControlClient &client_;
    const Network *network_{nullptr};
    ManagedNetwork *managed_network_{nullptr};
  };

} // namespace softadastra

#endif // SOFTADASTRA_CLI_CLI_HPP
