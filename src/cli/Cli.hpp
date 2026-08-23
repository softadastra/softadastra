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
#include "platform/Process.hpp"

namespace softadastra
{
  /**
   * @brief Provides the command-line interface for controlling a Host.
   *
   * Cli translates command-line arguments into operations performed through
   * ControlClient. It does not implement Host lifecycle, software lifecycle,
   * connectivity, or platform behavior itself.
   *
   * The Process reference represents the current execution capability used by
   * start and stop operations. Process creation and software-specific execution
   * configuration remain outside the CLI.
   */
  class Cli
  {
  public:
    /**
     * @brief Creates a command-line interface.
     *
     * The ControlClient and Process instances must remain valid for the lifetime
     * of the Cli instance.
     *
     * @param client Control client used to communicate with the Host.
     * @param process Process capability used for software lifecycle operations.
     */
    Cli(
        ControlClient &client,
        Process &process) noexcept;

    /**
     * @brief Executes a command-line request.
     *
     * Supported commands are:
     *
     * @code
     * softadastra register <software-id>
     * softadastra start <software-id>
     * softadastra stop <software-id>
     * softadastra status <software-id>
     * softadastra connectivity
     * softadastra help
     * @endcode
     *
     * @param argc Number of command-line arguments.
     * @param argv Command-line arguments.
     *
     * @return 0 on success, 1 when an operation fails, or 2 for invalid usage.
     */
    int run(
        int argc,
        const char *const argv[]);

  private:
    ControlClient &client_;
    Process &process_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CLI_CLI_HPP
