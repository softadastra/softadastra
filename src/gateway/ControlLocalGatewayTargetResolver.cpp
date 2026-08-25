#include "gateway/ControlLocalGatewayTargetResolver.hpp"

namespace softadastra
{
  ControlLocalGatewayTargetResolver::ControlLocalGatewayTargetResolver(ControlClient &client) noexcept
      : client_(client)
  {
  }

  LocalGatewayTarget ControlLocalGatewayTargetResolver::resolve(std::string_view host) const
  {
    if (!client_.host_available()) return {LocalGatewayLookup::Unavailable, 0};
    return client_.local_gateway_target(host);
  }
}
