/**
 *
 *  @file HostIdentity.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_IDENTITY_HPP
#define SOFTADASTRA_HOST_HOST_IDENTITY_HPP

#include <filesystem>
#include <string>

namespace softadastra
{
  /**
   * @brief Stores the persistent identifier and remote administration secret.
   */
  class HostIdentity
  {
  public:
    /** @brief Creates identity storage at an explicit local path. */
    explicit HostIdentity(std::filesystem::path path) noexcept;

    /** @brief Loads an existing identity or creates it once. */
    [[nodiscard]] bool load_or_create();

    /** @brief Returns the persistent public Host identifier. */
    [[nodiscard]] const std::string &id() const noexcept;

    /** @brief Returns the remote administration secret. */
    [[nodiscard]] const std::string &secret() const noexcept;

  private:
    std::filesystem::path path_;
    std::string id_;
    std::string secret_;
  };
}

#endif // SOFTADASTRA_HOST_HOST_IDENTITY_HPP
