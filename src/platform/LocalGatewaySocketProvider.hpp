#ifndef SOFTADASTRA_PLATFORM_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
namespace softadastra { enum class LocalGatewaySocketState { Available, Unavailable, Invalid }; struct LocalGatewaySocket { LocalGatewaySocketState state{LocalGatewaySocketState::Unavailable}; int fd{-1}; }; class LocalGatewaySocketProvider { public: virtual ~LocalGatewaySocketProvider()=default; [[nodiscard]] virtual LocalGatewaySocket acquire() const noexcept=0; }; }
#endif
