/**
 *
 *  @file ControlLocalGatewayTargetResolver.cpp
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

#include "gateway/ControlLocalGatewayTargetResolver.hpp"

#include "host/HostObservation.hpp"
#include "platform/NativeDataDirectory.hpp"

namespace softadastra
{
  ControlLocalGatewayTargetResolver::ControlLocalGatewayTargetResolver(
      ControlClient &client) noexcept
      : client_(client)
  {
  }

  LocalGatewayTarget ControlLocalGatewayTargetResolver::resolve(
      std::string_view host) const
  {
    if (!observe_host(
             client_,
             NativeDataDirectory::path())
             .available())
    {
      return {
          LocalGatewayLookup::Unavailable,
          0};
    }

    return client_.local_gateway_target(host);
  }

} // namespace softadastra
