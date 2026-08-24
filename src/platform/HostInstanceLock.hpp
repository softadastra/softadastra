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

namespace softadastra
{
  /**
   * @brief Holds the advisory runtime lock for one Host data directory.
   */
  class HostInstanceLock
  {
  public:
    HostInstanceLock() = default;
    ~HostInstanceLock();
    HostInstanceLock(const HostInstanceLock &) = delete;
    HostInstanceLock &operator=(const HostInstanceLock &) = delete;

    /**
     * @brief Acquires the non-stale exclusive lock for a data directory.
     */
    [[nodiscard]] bool acquire(const std::filesystem::path &directory) noexcept;
    [[nodiscard]] static bool is_held(const std::filesystem::path &directory) noexcept;

  private:
    int descriptor_{-1};
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_HOST_INSTANCE_LOCK_HPP
