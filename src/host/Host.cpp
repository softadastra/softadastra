/**
 *
 *  @file Host.cpp
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

#include "host/Host.hpp"

namespace softadastra
{
  Host::Host(Platform &platform) noexcept
      : platform_(platform)
  {
  }

  HostState &Host::state() noexcept
  {
    return state_;
  }

  const HostState &Host::state() const noexcept
  {
    return state_;
  }

  Platform &Host::platform() noexcept
  {
    return platform_;
  }

  const Platform &Host::platform() const noexcept
  {
    return platform_;
  }

} // namespace softadastra
