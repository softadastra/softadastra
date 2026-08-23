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

namespace softadastra
{
  /**
   * @brief Provides the Softadastra command-line interface.
   *
   * Cli translates command-line arguments into Host control operations through
   * ControlClient.
   *
   * It does not interact directly with platform processes or other
   * operating-system capabilities.
   */
  class Cli
  {
  public:
    /**
     * @brief Creates a command-line interface.
     *
     * @param client Control client used to communicate with the Host.
     */
    explicit Cli(ControlClient &client) noexcept;

    /**
     * @brief Executes a command-line operation.
     *
     * Supported commands:
     *
     * @code
     * softadastra register <software-id> <executable> [arguments...]
     * softadastra start <software-id>
     * softadastra stop <software-id>
     * softadastra restart <software-id>
     * softadastra status <software-id>
     * softadastra status
     * softadastra connectivity
     * softadastra access [port]
     * softadastra help
     * @endcode
     *
     * @param argc Number of command-line arguments.
     * @param argv Command-line arguments.
     *
     * @return 0 on success, 1 when an operation fails, or 2 when command usage
     *         is invalid.
     */
    int run(int argc, const char *const argv[]);

  private:
    ControlClient &client_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CLI_CLI_HPP
