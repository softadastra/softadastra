#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
#include "platform/LocalGatewaySocketProvider.hpp"
#include <functional>
namespace softadastra { class NativeLocalGatewaySocketProvider final : public LocalGatewaySocketProvider { public: using Environment=std::function<const char *(const char *)>; explicit NativeLocalGatewaySocketProvider(Environment environment = {} ) noexcept; [[nodiscard]] LocalGatewaySocket acquire() const noexcept override; private: Environment environment_; }; }
#endif
