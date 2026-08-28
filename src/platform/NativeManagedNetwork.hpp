/**
 *
 *  @file NativeManagedNetwork.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_MANAGED_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_MANAGED_NETWORK_HPP

#include "platform/ManagedNetwork.hpp"
#include "platform/Network.hpp"

#include <string>
#include <vector>

namespace softadastra
{
  /**
   * @brief Represents the result of an nmcli command.
   */
  struct NmcliResult
  {
    /**
     * @brief Process exit code.
     */
    int code{-1};

    /**
     * @brief Combined command output.
     */
    std::string output;
  };

  /**
   * @brief Provides an interface for executing nmcli commands.
   */
  class NmcliRunner
  {
  public:
    virtual ~NmcliRunner() = default;

    /**
     * @brief Executes nmcli with the supplied arguments.
     *
     * @param arguments Command arguments.
     *
     * @return Result of the command execution.
     */
    [[nodiscard]] virtual NmcliResult run(
        const std::vector<std::string> &arguments) = 0;
  };

  /**
   * @brief Implements managed networking using native platform facilities.
   */
  class NativeManagedNetwork final : public ManagedNetwork
  {
  public:
    /**
     * @brief Creates a native managed network.
     *
     * @param runner Optional nmcli command runner.
     * @param network Optional network capability provider.
     */
    explicit NativeManagedNetwork(
        NmcliRunner *runner = nullptr,
        const Network *network = nullptr) noexcept
        : runner_(runner),
          network_(network)
    {
    }

    /**
     * @brief Returns the current managed network status.
     *
     * @return Current managed network status.
     */
    [[nodiscard]] ManagedNetworkStatus status() const override;

    /**
     * @brief Starts the managed network.
     *
     * @return Result of the start operation.
     */
    [[nodiscard]] ManagedNetworkStartResult start() override;

    /**
     * @brief Stops the managed network.
     *
     * @return true if the managed network was stopped successfully,
     *         otherwise false.
     */
    bool stop() override;

  private:
    NmcliRunner *runner_;
    const Network *network_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_MANAGED_NETWORK_HPP
