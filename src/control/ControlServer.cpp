/**
 *
 *  @file ControlServer.cpp
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

#include "control/ControlServer.hpp"
#include <utility>

namespace softadastra
{
  ControlServer::ControlServer(HostService &host_service) noexcept
      : host_service_(host_service)
  {
  }

  bool ControlServer::register_software(SoftwareId id)
  {
    return host_service_.register_software(
        std::move(id));
  }

  bool ControlServer::start_software(
      const SoftwareId &id,
      Process &process)
  {
    return host_service_.start_software(
        id,
        process);
  }

  bool ControlServer::stop_software(
      const SoftwareId &id,
      Process &process)
  {
    return host_service_.stop_software(
        id,
        process);
  }

  std::optional<SoftwareState> ControlServer::software_state(
      const SoftwareId &id) const noexcept
  {
    return host_service_.software_state(id);
  }

  bool ControlServer::connectivity_available() const noexcept
  {
    return host_service_.connectivity_available();
  }

  bool ControlServer::connected() const noexcept
  {
    return host_service_.connected();
  }

} // namespace softadastra
