/**
 *
 *  @file HostProfile.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_PROFILE_HPP
#define SOFTADASTRA_HOST_HOST_PROFILE_HPP

#include "host/LocalReachability.hpp"
#include "platform/ManagedNetwork.hpp"

#include <filesystem>
#include <string>

namespace softadastra
{
  /**
   * @brief Defines the provisioning profile of a Host.
   */
  enum class HostProfile
  {
    Standard,
    Box
  };

  /**
   * @brief Describes the operational state of a provisioned Softadastra Box.
   */
  enum class BoxState
  {
    NotProvisioned,
    Stopped,
    Ready,
    Degraded
  };

  /**
   * @brief Stores the provisioning profile associated with a persistent Host identity.
   */
  class HostProfileStore
  {
  public:
    /**
     * @brief Creates a Host profile store.
     *
     * @param path Path to the persistent profile file.
     */
    explicit HostProfileStore(
        std::filesystem::path path) noexcept;

    /**
     * @brief Loads the profile associated with a Host identity.
     *
     * A missing profile file represents the Standard profile.
     *
     * @param host_id Persistent Host identifier.
     *
     * @return true if the profile was loaded successfully or no profile file
     *         exists, otherwise false.
     */
    [[nodiscard]] bool load(
        const std::string &host_id);

    /**
     * @brief Provisions the Host with the Box profile.
     *
     * @param host_id Persistent Host identifier associated with the profile.
     *
     * @return true if the Box profile was persisted successfully,
     *         otherwise false.
     */
    [[nodiscard]] bool provision_box(
        const std::string &host_id);

    /**
     * @brief Removes the persisted provisioning profile.
     *
     * @return true if the profile was removed successfully, otherwise false.
     */
    [[nodiscard]] bool unprovision();

    /**
     * @brief Returns the currently loaded Host profile.
     *
     * @return Current Host profile.
     */
    [[nodiscard]] HostProfile profile() const noexcept;

  private:
    std::filesystem::path path_;
    HostProfile profile_{HostProfile::Standard};
  };

  /**
   * @brief Returns the canonical name of a Host profile.
   *
   * @param profile Host profile.
   *
   * @return Canonical profile name.
   */
  [[nodiscard]] const char *host_profile_name(
      HostProfile profile) noexcept;

  /**
   * @brief Determines the operational state of a Softadastra Box.
   *
   * @param profile Current Host profile.
   * @param host_running Whether the Host is currently running.
   * @param managed_network Current managed network status.
   * @param reachability Current local reachability state.
   *
   * @return Current Box state.
   */
  [[nodiscard]] BoxState box_state(
      HostProfile profile,
      bool host_running,
      ManagedNetworkStatus managed_network,
      LocalReachabilityState reachability =
          LocalReachabilityState::Ready) noexcept;

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_PROFILE_HPP
