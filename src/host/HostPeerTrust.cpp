/**
 *
 *  @file HostPeerTrust.cpp
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

#include "host/HostPeerTrust.hpp"

#include "host/HostPeerIdentity.hpp"

#include <utility>

namespace softadastra
{
  HostPeerTrust::HostPeerTrust(
      std::string address,
      std::string expected_id) noexcept
      : address_(std::move(address)),
        expected_id_(std::move(expected_id))
  {
  }

  const std::string &HostPeerTrust::address() const noexcept
  {
    return address_;
  }

  const std::string &HostPeerTrust::expected_id() const noexcept
  {
    return expected_id_;
  }

  bool HostPeerTrust::accepts(std::string_view presented_id) const noexcept
  {
    return HostPeerIdentity::matches(expected_id_, presented_id);
  }

} // namespace softadastra
