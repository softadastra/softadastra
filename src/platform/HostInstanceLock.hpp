/**
 *
 *  @file HostInstanceLock.hpp
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

#ifndef SOFTADASTRA_PLATFORM_HOST_INSTANCE_LOCK_HPP
#define SOFTADASTRA_PLATFORM_HOST_INSTANCE_LOCK_HPP

#include <filesystem>

#if defined(_WIN32)

#include <windows.h>

#endif

namespace softadastra
{
  enum class HostInstanceLockState
  {
    Free,
    Held,
    Error
  };

  /**
   * @brief Holds the runtime lock for one Host data directory.
   *
   * HostInstanceLock prevents multiple Host instances from acquiring the
   * same data directory simultaneously.
   */
  class HostInstanceLock
  {
  public:
    /**
     * @brief Creates an unlocked Host instance lock.
     */
    HostInstanceLock() = default;

    /**
     * @brief Releases the acquired Host instance lock.
     */
    ~HostInstanceLock();

    HostInstanceLock(const HostInstanceLock &) = delete;
    HostInstanceLock &operator=(const HostInstanceLock &) = delete;

    /**
     * @brief Acquires the exclusive lock for a Host data directory.
     *
     * @param directory Host data directory to lock.
     *
     * @return true if the lock was acquired or is already held by this
     *         instance, otherwise false.
     */
    [[nodiscard]] bool acquire(
        const std::filesystem::path &directory) noexcept;

    /** @brief Probes the current lock state without retaining the lock. */
    [[nodiscard]] static HostInstanceLockState probe(
        const std::filesystem::path &directory) noexcept;

    /** @brief Checks whether the Host data directory is currently locked. */
    [[nodiscard]] static bool is_held(
        const std::filesystem::path &directory) noexcept;

  private:
    int descriptor_{-1};

#if defined(_WIN32)
    HANDLE mutex_{nullptr};
#endif
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_HOST_INSTANCE_LOCK_HPP
