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

  bool ControlClient::register_software(
      SoftwareId id,
      ProcessSpec process_spec)
  {
    return server_.register_software(
        std::move(id),
        std::move(process_spec));
  }

  bool ControlClient::start_software(const SoftwareId &id)
  {
    return server_.start_software(id);
  }

  bool ControlClient::stop_software(const SoftwareId &id)
  {
    return server_.stop_software(id);
  }

  void ControlClient::refresh()
  {
    server_.refresh();
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
