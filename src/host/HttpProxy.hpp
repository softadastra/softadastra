/**
 *
 *  @file HttpProxy.hpp
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

#ifndef SOFTADASTRA_HOST_HTTP_PROXY_HPP
#define SOFTADASTRA_HOST_HTTP_PROXY_HPP

#include "host/LocalGatewayTargetResolver.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace softadastra
{
  /**
   * @brief Represents an HTTP request forwarded by the local proxy.
   */
  struct HttpProxyRequest
  {
    /**
     * @brief HTTP request method.
     */
    std::string method;

    /**
     * @brief HTTP request target.
     */
    std::string target;

    /**
     * @brief HTTP request headers.
     */
    std::vector<std::pair<std::string, std::string>> headers;

    /**
     * @brief HTTP request body.
     */
    std::string body;
  };

  /**
   * @brief Provides the outbound HTTP/1.1 proxy core for local software.
   *
   * Target selection is delegated to LocalGatewayTargetResolver.
   */
  class HttpProxy
  {
  public:
    /**
     * @brief Creates an HTTP proxy.
     *
     * @param resolver Resolver used to locate the target software endpoint.
     */
    explicit HttpProxy(
        LocalGatewayTargetResolver &resolver) noexcept
        : resolver_(resolver)
    {
    }

    /**
     * @brief Forwards an HTTP request to registered local software.
     *
     * @param software Software host name to resolve.
     * @param request HTTP request to forward.
     *
     * @return Raw HTTP response produced by the target or by the proxy.
     */
    [[nodiscard]] std::string forward(
        std::string_view software,
        const HttpProxyRequest &request) const;

  private:
    LocalGatewayTargetResolver &resolver_;
  };

} // namespace softadastra

#endif // SOFTADASTRA_HOST_HTTP_PROXY_HPP
