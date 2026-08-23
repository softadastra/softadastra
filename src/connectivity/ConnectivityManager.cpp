/**
 *
 *  @file ConnectivityManager.cpp
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

#include "connectivity/ConnectivityManager.hpp"

namespace softadastra
{
  ConnectivityManager::ConnectivityManager(Network &network) noexcept
      : network_(network)
  {
  }

  bool ConnectivityManager::is_available() const noexcept
  {
    return network_.is_available();
  }

  bool ConnectivityManager::is_connected() const noexcept
  {
    if (!network_.is_available())
    {
      return false;
    }

    return network_.is_connected();
  }

} // namespace softadastra
