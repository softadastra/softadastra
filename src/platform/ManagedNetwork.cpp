/**
 *
 *  @file ManagedNetwork.cpp
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

#include "platform/ManagedNetwork.hpp"

namespace softadastra
{
  const char *managed_network_state_name(
      ManagedNetworkState state) noexcept
  {
    return state == ManagedNetworkState::Running
               ? "running"
               : "stopped";
  }

} // namespace softadastra
