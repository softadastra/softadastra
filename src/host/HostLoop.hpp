/**
 *
 *  @file HostLoop.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_LOOP_HPP
#define SOFTADASTRA_HOST_HOST_LOOP_HPP

#include "host/HostService.hpp"
#include "host/HostStateFile.hpp"

#include <chrono>
#include <condition_variable>
#include <mutex>

namespace softadastra
{
  /**
   * @brief Runs the persistent Host lifecycle in the current process.
   *
   * HostLoop restores registrations before entering its wait loop. It refreshes
   * managed software at a bounded interval until an internal stop request is
   * received. It then stops managed processes and persists registrations.
   */
  class HostLoop
  {
  public:
    /**
     * @brief Creates a Host lifecycle loop.
     *
     * @param host_service Host lifecycle operations used for supervision.
     * @param state_file Persistent Host registration metadata.
     * @param interval Maximum delay between supervision cycles.
     */
    HostLoop(
        HostService &host_service,
        HostStateFile &state_file,
        std::chrono::milliseconds interval = std::chrono::seconds(1)) noexcept;

    /**
     * @brief Restores state and runs until a stop request is received.
     *
     * @return true when the loop exits after a stop request, otherwise false
     *         when state restoration fails or another loop is already active.
     */
    [[nodiscard]] bool run();

    /**
     * @brief Requests that the active loop exit promptly.
     */
    void request_stop() noexcept;

    /**
     * @brief Returns whether the lifecycle loop is active.
     */
    [[nodiscard]] bool is_running() const noexcept;

  private:
    [[nodiscard]] bool shutdown();

    HostService &host_service_;
    HostStateFile &state_file_;
    std::chrono::milliseconds interval_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    bool running_{false};
    bool stop_requested_{false};
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_LOOP_HPP
