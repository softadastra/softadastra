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
  /**
   * @brief Describes the result of a local gateway target lookup.
   */
  enum class LocalGatewayLookup
  {
    NotFound,
    Unavailable,
    Http
  };

  /**
   * @brief Represents the result of resolving a local gateway target.
   */
  struct LocalGatewayTarget
  {
    /**
     * @brief Result of the target lookup.
     */
    LocalGatewayLookup result{
        LocalGatewayLookup::NotFound};

    /**
     * @brief Port exposed by the resolved target.
     */
    std::uint16_t port{};
  };

  /**
   * @brief Provides the interface for resolving local gateway targets.
   */
  class LocalGatewayTargetResolver
  {
  public:
    /**
     * @brief Destroys the local gateway target resolver.
     */
    virtual ~LocalGatewayTargetResolver() = default;

    /**
     * @brief Resolves a local host name to a gateway target.
     *
     * @param host Host name to resolve.
     *
     * @return Resolved local gateway target.
     */
    [[nodiscard]] virtual LocalGatewayTarget resolve(
        std::string_view host) const = 0;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_LOCAL_GATEWAY_TARGET_RESOLVER_HPP
