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
   * @brief Represents a process managed through the native process backend.
   *
   * NativeProcess provides the platform-specific implementation of Process
   * while supporting processes represented by Vix child handles or native
   * operating system process identifiers.
   */
  class NativeProcess final : public Process
  {
  public:
    using Id = vix::process::ProcessId;

    /**
     * @brief Creates a native process from a Vix child handle.
     *
     * @param child Child process handle.
     */
    explicit NativeProcess(vix::process::Child child) noexcept;

    /**
     * @brief Releases native process resources.
     */
    ~NativeProcess() override;

#if defined(__linux__)

    /**
     * @brief Creates a native process from a Linux process identifier.
     *
     * @param pid Native process identifier.
     */
    explicit NativeProcess(pid_t pid) noexcept;

    /**
     * @brief Returns the Linux process identifier.
     *
     * @return Native process identifier when available, or std::nullopt
     *         otherwise.
     */
    [[nodiscard]] std::optional<pid_t> native_pid() const noexcept;

#endif

#if defined(_WIN32)

    /**
     * @brief Creates a native process from Windows process handles.
     *
     * @param process Process handle.
     * @param job Job object handle associated with the process.
     * @param pid Native process identifier.
     */
    NativeProcess(
        HANDLE process,
        HANDLE job,
        DWORD pid) noexcept;

#endif

    /**
     * @brief Requests termination of the process.
     *
     * @return true if the process was stopped successfully, otherwise false.
     */
    bool stop() override;

    /**
     * @brief Returns whether the process is currently running.
     *
     * @return true if the process is running, otherwise false.
     */
    [[nodiscard]] bool is_running() const noexcept override;

    /**
     * @brief Returns the process exit code when available.
     *
     * @return Exit code when the process has terminated, or std::nullopt
     *         otherwise.
     */
    [[nodiscard]] std::optional<int> exit_code() noexcept override;

    /**
     * @brief Returns the Vix process identifier.
     *
     * @return Process identifier associated with the child handle.
     */
    [[nodiscard]] Id id() const noexcept;

    /**
     * @brief Returns the process identifier in the generic Process format.
     *
     * @return Process identifier when available, or std::nullopt otherwise.
     */
    [[nodiscard]] std::optional<long> pid() const noexcept override;

  private:
    vix::process::Child child_;

#if defined(__linux__)
    pid_t native_pid_{-1};
#elif defined(_WIN32)
    DWORD native_pid_{0};
#else
    long native_pid_{-1};
#endif

    std::optional<int> exit_code_;

#if defined(_WIN32)
    HANDLE process_{nullptr};
    HANDLE job_{nullptr};
#endif
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_PROCESS_HPP
