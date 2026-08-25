/**
 * @file ControlLocalGatewayTargetResolver.hpp
 * @brief Resolves local gateway targets through the Host control endpoint.
 */
#ifndef SOFTADASTRA_GATEWAY_CONTROL_LOCAL_GATEWAY_TARGET_RESOLVER_HPP
#define SOFTADASTRA_GATEWAY_CONTROL_LOCAL_GATEWAY_TARGET_RESOLVER_HPP

#include "control/ControlClient.hpp"
#include "host/LocalGatewayTargetResolver.hpp"

namespace softadastra
{
  class ControlLocalGatewayTargetResolver final : public LocalGatewayTargetResolver
  {
  public:
    explicit ControlLocalGatewayTargetResolver(ControlClient &client) noexcept;
    [[nodiscard]] LocalGatewayTarget resolve(std::string_view host) const override;

  private:
    ControlClient &client_;
  };
}

#endif
