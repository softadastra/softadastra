/**
 *
 *  @file HostObservation.hpp
 *  @brief Canonical observation of a local Host instance.
 */

#ifndef SOFTADASTRA_HOST_HOST_OBSERVATION_HPP
#define SOFTADASTRA_HOST_HOST_OBSERVATION_HPP

#include <filesystem>

namespace softadastra
{
  class ControlClient;

  enum class HostAvailability
  {
    Running,
    Stopped,
    Unavailable,
    Unknown
  };

  struct HostObservation
  {
    HostAvailability state{HostAvailability::Unknown};

    [[nodiscard]] bool available() const noexcept
    {
      return state == HostAvailability::Running;
    }
  };

  [[nodiscard]] HostObservation observe_host(
      const ControlClient &client,
      const std::filesystem::path &data_directory) noexcept;

  [[nodiscard]] const char *host_availability_name(
      HostAvailability state) noexcept;

  /**
   * @brief Returns whether a Box observation permits a user Host to start.
   *
   * A Box which is running, unavailable, or cannot be inspected must retain
   * the machine until its state is known to be stopped.
   */
  [[nodiscard]] bool box_allows_user_host_start(
      HostObservation observation) noexcept;
} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_OBSERVATION_HPP
