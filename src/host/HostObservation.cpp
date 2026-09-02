/**
 *
 *  @file HostObservation.cpp
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

#include "host/HostObservation.hpp"

#include "control/ControlClient.hpp"
#include "platform/HostInstanceLock.hpp"

namespace softadastra
{
  HostObservation observe_host(
      const ControlClient &client,
      const std::filesystem::path &data_directory) noexcept
  {
    if (client.host_available())
    {
      return {
          HostAvailability::Running};
    }

    switch (
        HostInstanceLock::probe(
            data_directory))
    {
    case HostInstanceLockState::Free:
      return {
          HostAvailability::Stopped};

    case HostInstanceLockState::Held:
      return {
          HostAvailability::Unavailable};

    case HostInstanceLockState::Error:
      return {
          HostAvailability::Unknown};
    }

    return {
        HostAvailability::Unknown};
  }

  const char *host_availability_name(
      HostAvailability state) noexcept
  {
    switch (state)
    {
    case HostAvailability::Running:
      return "running";

    case HostAvailability::Stopped:
      return "stopped";

    case HostAvailability::Unavailable:
      return "unavailable";

    case HostAvailability::Unknown:
      return "unknown";
    }

    return "unknown";
  }

  bool box_allows_user_host_start(
      HostObservation observation) noexcept
  {
    return observation.state ==
           HostAvailability::Stopped;
  }

} // namespace softadastra
