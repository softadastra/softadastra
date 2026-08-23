/**
 *
 *  @file ControlClient.cpp
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

#include "control/ControlClient.hpp"
#include <utility>

namespace softadastra
{
  ControlClient::ControlClient(ControlServer &server) noexcept
      : server_(server)
  {
  }

  bool ControlClient::register_software(SoftwareId id)
  {
    return server_.register_software(
        std::move(id));
  }

  bool ControlClient::start_software(
      const SoftwareId &id,
      Process &process)
  {
    return server_.start_software(
        id,
        process);
  }

  bool ControlClient::stop_software(
      const SoftwareId &id,
      Process &process)
  {
    return server_.stop_software(
        id,
        process);
  }

  std::optional<SoftwareState> ControlClient::software_state(
      const SoftwareId &id) const noexcept
  {
    return server_.software_state(id);
  }

  bool ControlClient::connectivity_available() const noexcept
  {
    return server_.connectivity_available();
  }

  bool ControlClient::connected() const noexcept
  {
    return server_.connected();
  }

} // namespace softadastra
