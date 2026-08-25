/**
 *
 *  @file LocalGatewayTargetResolver.hpp
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

#ifndef SOFTADASTRA_HOST_LOCAL_GATEWAY_TARGET_RESOLVER_HPP
#define SOFTADASTRA_HOST_LOCAL_GATEWAY_TARGET_RESOLVER_HPP

#include <cstdint>
#include <string_view>

namespace softadastra
{
  enum class LocalGatewayLookup { NotFound, Unavailable, Http };
  struct LocalGatewayTarget { LocalGatewayLookup result{LocalGatewayLookup::NotFound}; std::uint16_t port{}; };

  class LocalGatewayTargetResolver
  {
  public:
    virtual ~LocalGatewayTargetResolver() = default;
    [[nodiscard]] virtual LocalGatewayTarget resolve(std::string_view host) const = 0;
  };
}

#endif
