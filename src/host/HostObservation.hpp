/**
 *
 *  @file HostObservation.hpp
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

#ifndef SOFTADASTRA_HOST_HOST_OBSERVATION_HPP
#define SOFTADASTRA_HOST_HOST_OBSERVATION_HPP

#include <filesystem>

namespace softadastra
{
  class ControlClient;

  /**
   * @brief Describes the observed availability of a local Host instance.
   */
  enum class HostAvailability
  {
    Running,
    Stopped,
    Unavailable,
    Unknown
  };

  /**
   * @brief Represents the current observation of a local Host instance.
   */
  struct HostObservation
  {
    /**
     * @brief Observed Host availability.
     */
    HostAvailability state{
        HostAvailability::Unknown};

    /**
     * @brief Checks whether the Host is currently available.
     *
     * @return true when the Host is running, otherwise false.
     */
    [[nodiscard]] bool available() const noexcept
    {
      return state == HostAvailability::Running;
    }
  };

  /**
   * @brief Observes the availability of the local Host.
   *
   * @param client Control client used to probe the running Host.
   * @param data_directory Host data directory used to inspect the instance lock.
   *
   * @return Current Host observation.
   */
  [[nodiscard]] HostObservation observe_host(
      const ControlClient &client,
      const std::filesystem::path &data_directory) noexcept;

  /**
   * @brief Returns the canonical name of a Host availability state.
   *
   * @param state Host availability state.
   *
   * @return Canonical availability name.
   */
  [[nodiscard]] const char *host_availability_name(
      HostAvailability state) noexcept;

  /**
   * @brief Returns whether a Box observation permits a user Host to start.
   *
   * A Box which is running, unavailable, or cannot be inspected must retain
   * the machine until its state is known to be stopped.
   *
   * @param observation Current Box Host observation.
   *
   * @return true only when the observed Host is stopped.
   */
  [[nodiscard]] bool box_allows_user_host_start(
      HostObservation observation) noexcept;

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_OBSERVATION_HPP
