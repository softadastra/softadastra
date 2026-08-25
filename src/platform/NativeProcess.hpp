/**
 *
 *  @file NativeProcess.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP

#include "platform/Process.hpp"

#include <optional>
#if defined(__linux__)
#include <sys/types.h>
#endif
#if defined(_WIN32)
#include <windows.h>
#endif
#include <vix/process/Child.hpp>

namespace softadastra
{

  /**
   * @brief Represents a process launched through the native process backend.
   */
  class NativeProcess final : public Process
  {
  public:
    using Id = vix::process::ProcessId;

    /**
     * @brief Creates a native process from a Vix child handle.
     */
    explicit NativeProcess(vix::process::Child child) noexcept;
    ~NativeProcess() override;
#if defined(__linux__)
    explicit NativeProcess(pid_t pid) noexcept;
#endif
#if defined(_WIN32)
    NativeProcess(HANDLE process, HANDLE job, DWORD pid) noexcept;
#endif

    /**
     * @brief Requests graceful termination of the process.
     */
    bool stop() override;

    /**
     * @brief Returns whether the process is currently running.
     */
    [[nodiscard]] bool is_running() const noexcept override;

    /**
     * @brief Returns the exit code when the process has terminated.
     */
    [[nodiscard]] std::optional<int> exit_code() noexcept override;

    /**
     * @brief Returns the native process identifier.
     */
    [[nodiscard]] Id id() const noexcept;
    [[nodiscard]] std::optional<long> pid() const noexcept override;

  private:
    vix::process::Child child_;
    long native_pid_{-1};
    std::optional<int> exit_code_;
#if defined(_WIN32)
    HANDLE process_{nullptr};
    HANDLE job_{nullptr};
#endif
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP
