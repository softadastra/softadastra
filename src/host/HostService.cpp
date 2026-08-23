/**
 *
 *  @file HostService.cpp
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

#include "host/HostService.hpp"

#include <utility>

namespace softadastra
{

  HostService::HostService(
      Host &host,
      ProcessLauncher &process_launcher) noexcept
      : host_(host),
        software_manager_(host.state(), process_launcher),
        connectivity_manager_(host.platform().network())
  {
  }

  Host &HostService::host() noexcept
  {
    return host_;
  }

  const Host &HostService::host() const noexcept
  {
    return host_;
  }

  bool HostService::register_software(
      SoftwareId id,
      ProcessSpec process_spec)
  {
    return software_manager_.register_software(
        std::move(id),
        std::move(process_spec));
  }

  bool HostService::start_software(const SoftwareId &id)
  {
    return software_manager_.start(id);
  }

  bool HostService::stop_software(const SoftwareId &id)
  {
    return software_manager_.stop(id);
  }

  std::optional<SoftwareState> HostService::software_state(
      const SoftwareId &id) const noexcept
  {
    return software_manager_.state(id);
  }

  bool HostService::connectivity_available() const noexcept
  {
    return connectivity_manager_.is_available();
  }

  bool HostService::connected() const noexcept
  {
    return connectivity_manager_.is_connected();
  }

} // namespace softadastra
