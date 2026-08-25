#ifndef SOFTADASTRA_HOST_HTTP_PROXY_HPP
#define SOFTADASTRA_HOST_HTTP_PROXY_HPP

#include "host/LocalGatewayTargetResolver.hpp"
#include <string>
#include <utility>
#include <vector>

namespace softadastra {
struct HttpProxyRequest {
  std::string method;
  std::string target;
  std::vector<std::pair<std::string, std::string>> headers;
  std::string body;
};

/** Shared, outbound-only HTTP/1.1 proxy core.  The resolver owns target choice. */
class HttpProxy {
public:
  explicit HttpProxy(LocalGatewayTargetResolver &resolver) noexcept : resolver_(resolver) {}
  [[nodiscard]] std::string forward(std::string_view software, const HttpProxyRequest &request) const;
private:
  LocalGatewayTargetResolver &resolver_;
};
}
#endif
