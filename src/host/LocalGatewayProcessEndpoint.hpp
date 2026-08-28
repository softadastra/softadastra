/**
 *
 *  @file LocalGatewayProcessEndpoint.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_GATEWAY_PROCESS_ENDPOINT_HPP
#define SOFTADASTRA_HOST_LOCAL_GATEWAY_PROCESS_ENDPOINT_HPP

#include "host/LocalReachability.hpp"
#include "platform/ProcessLauncher.hpp"

#include <filesystem>
#include <memory>

namespace softadastra
{
  /**
   * @brief Runs the local gateway as a separately managed process.
   */
  class LocalGatewayProcessEndpoint final : public LocalGatewayEndpoint
  {
  public:
    /**
     * @brief Creates a process-backed local gateway endpoint.
     *
     * @param launcher Process launcher used to start the gateway.
     * @param executable Path to the gateway executable.
     * @param control Path to the gateway control endpoint.
     */
    LocalGatewayProcessEndpoint(
        ProcessLauncher &launcher,
        std::filesystem::path executable,
        std::filesystem::path control) noexcept;

    /**
     * @brief Starts the local gateway process.
     *
     * @param address Address on which the gateway should listen.
     * @param port Port on which the gateway should listen.
     *
     * @return true if the gateway process is running, otherwise false.
     */
    bool start(
        std::string address,
        std::uint16_t port) override;

    /**
     * @brief Stops the local gateway process.
     */
    void stop() noexcept override;

  private:
    ProcessLauncher &launcher_;
    std::filesystem::path executable_;
    std::filesystem::path control_;
    std::unique_ptr<Process> process_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_GATEWAY_PROCESS_ENDPOINT_HPP
