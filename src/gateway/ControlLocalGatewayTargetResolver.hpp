/**
 *
 *  @file ControlLocalGatewayTargetResolver.hpp
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

#ifndef SOFTADASTRA_GATEWAY_CONTROL_LOCAL_GATEWAY_TARGET_RESOLVER_HPP
#define SOFTADASTRA_GATEWAY_CONTROL_LOCAL_GATEWAY_TARGET_RESOLVER_HPP

#include "control/ControlClient.hpp"
#include "host/LocalGatewayTargetResolver.hpp"

namespace softadastra
{
  /**
   * @brief Resolves local gateway targets through the Host control interface.
   *
   * ControlLocalGatewayTargetResolver delegates local gateway target
   * resolution to a ControlClient connected to the Host control endpoint.
   */
  class ControlLocalGatewayTargetResolver final
      : public LocalGatewayTargetResolver
  {
  public:
    /**
     * @brief Creates a resolver backed by a Host control client.
     *
     * @param client Control client used to resolve gateway targets.
     */
    explicit ControlLocalGatewayTargetResolver(
        ControlClient &client) noexcept;

    /**
     * @brief Resolves a host name to its local gateway target.
     *
     * @param host Host name to resolve.
     *
     * @return Local gateway target associated with the host.
     */
    [[nodiscard]] LocalGatewayTarget resolve(
        std::string_view host) const override;

  private:
    ControlClient &client_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_GATEWAY_CONTROL_LOCAL_GATEWAY_TARGET_RESOLVER_HPP
