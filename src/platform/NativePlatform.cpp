/**
 *
 *  @file NativePlatform.cpp
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

#include "platform/NativePlatform.hpp"

namespace softadastra
{
  ProcessLauncher &NativePlatform::process_launcher() noexcept
  {
    return process_launcher_;
  }

  const ProcessLauncher &NativePlatform::process_launcher() const noexcept
  {
    return process_launcher_;
  }

  Service &NativePlatform::service() noexcept
  {
    return service_;
  }

  const Service &NativePlatform::service() const noexcept
  {
    return service_;
  }

  Network &NativePlatform::network() noexcept
  {
    return network_;
  }

  const Network &NativePlatform::network() const noexcept
  {
    return network_;
  }

  LocalFirewall &NativePlatform::local_firewall() noexcept
  {
    return local_firewall_;
  }

  const LocalFirewall &NativePlatform::local_firewall() const noexcept
  {
    return local_firewall_;
  }

  ManagedNetwork &NativePlatform::managed_network() noexcept
  {
    return managed_network_;
  }

  const ManagedNetwork &NativePlatform::managed_network() const noexcept
  {
    return managed_network_;
  }

} // namespace softadastra
