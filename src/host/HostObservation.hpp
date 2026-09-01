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
} // namespace softadastra

#endif // SOFTADASTRA_HOST_HOST_OBSERVATION_HPP
