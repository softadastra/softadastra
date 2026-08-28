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
  /**
   * @brief Defines persistent remote access settings for an outbound relay.
   */
  struct RemoteAccessSettings
  {
    /**
     * @brief Indicates whether remote access is enabled.
     */
    bool enabled{false};

    /**
     * @brief Address of the outbound relay.
     */
    std::string address;

    /**
     * @brief Port used to reach the outbound relay.
     */
    std::uint16_t port{0};
  };

  /**
   * @brief Manages persistent Host-local remote access configuration.
   *
   * RemoteAccessConfig loads and stores RemoteAccessSettings using an
   * explicitly configured local filesystem path.
   */
  class RemoteAccessConfig
  {
  public:
    /**
     * @brief Creates configuration storage at an explicit local path.
     *
     * @param path Filesystem path used to store the configuration.
     */
    explicit RemoteAccessConfig(std::filesystem::path path) noexcept;

    /**
     * @brief Loads the stored remote access configuration.
     *
     * @param settings Receives the loaded configuration.
     *
     * @return true if the configuration was loaded successfully, otherwise false.
     */
    [[nodiscard]] bool load(RemoteAccessSettings &settings) const;

    /**
     * @brief Persists the remote access configuration.
     *
     * @param settings Configuration to store.
     *
     * @return true if the configuration was saved successfully, otherwise false.
     */
    [[nodiscard]] bool save(const RemoteAccessSettings &settings) const;

  private:
    std::filesystem::path path_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_CONTROL_REMOTE_ACCESS_CONFIG_HPP
