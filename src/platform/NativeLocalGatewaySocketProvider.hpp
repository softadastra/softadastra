/**
 *
 *  @file NativeLocalGatewaySocketProvider.hpp
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

#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP

#include "platform/LocalGatewaySocketProvider.hpp"

#include <functional>

namespace softadastra
{
  /**
   * @brief Acquires a local gateway socket supplied by the native platform.
   */
  class NativeLocalGatewaySocketProvider final
      : public LocalGatewaySocketProvider
  {
  public:
    using Environment =
        std::function<const char *(const char *)>;

    /**
     * @brief Creates a native local gateway socket provider.
     *
     * @param environment Optional environment lookup function.
     */
    explicit NativeLocalGatewaySocketProvider(
        Environment environment = {}) noexcept;

    /**
     * @brief Acquires the local gateway socket from the native runtime environment.
     *
     * @return Local gateway socket acquisition result.
     */
    [[nodiscard]] LocalGatewaySocket acquire() const noexcept override;

  private:
    Environment environment_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_PLATFORM_NATIVE_LOCAL_GATEWAY_SOCKET_PROVIDER_HPP
