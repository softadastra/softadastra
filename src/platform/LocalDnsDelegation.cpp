#include "platform/LocalDnsDelegation.hpp"
#include "platform/LocalDnsConfiguration.hpp"
namespace softadastra { std::string LocalDnsDelegation::configuration() { return std::string("server=/") + local_dns_zone + "/127.0.0.1#" + std::to_string(local_dns_port) + "\n"; } }
