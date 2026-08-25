/**
 *
 *  @file RemoteAccessConfig.hpp
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

#ifndef SOFTADASTRA_CONTROL_REMOTE_ACCESS_CONFIG_HPP
#define SOFTADASTRA_CONTROL_REMOTE_ACCESS_CONFIG_HPP

#include <cstdint>
#include <filesystem>
#include <string>

namespace softadastra
{
  /** @brief Persistent, explicitly configured outbound relay settings. */
  struct RemoteAccessSettings
  {
    bool enabled{false};
    std::string address; // relay address; never a local bind address
    std::uint16_t port{0};
  };

  /** @brief Stores remote access settings in Host-local state. */
  class RemoteAccessConfig
  {
  public:
    /** @brief Creates configuration storage at an explicit local path. */
    explicit RemoteAccessConfig(std::filesystem::path path) noexcept;

    /** @brief Loads an existing configuration. */
    [[nodiscard]] bool load(RemoteAccessSettings &settings) const;

    /** @brief Persists a configuration. */
    [[nodiscard]] bool save(const RemoteAccessSettings &settings) const;

  private:
    std::filesystem::path path_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_REMOTE_ACCESS_CONFIG_HPP
