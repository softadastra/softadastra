/*
 * discovery_listener.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY LISTENER EXAMPLE ==\n";

  auto config =
      discovery::core::DiscoveryConfig::local(
          "node-listener",
          9401,
          7001);

  discovery::backend::UdpDiscoveryBackend backend{config};
  discovery::server::DiscoveryServer server{backend};

  if (!server.start())
  {
    std::cerr << "failed to start discovery listener\n";
    return 1;
  }

  std::cout << "listener running on "
            << config.bind_host
            << ":"
            << config.bind_port
            << "\n";

  auto inbound =
      server.poll();

  if (!inbound.has_value())
  {
    std::cout << "no inbound discovery message available\n";
    server.stop();
    return 0;
  }

  std::cout << "received discovery message from: "
            << inbound->message.from_node_id
            << "\n";

  std::cout << "message type: "
            << discovery::types::to_string(inbound->message.type)
            << "\n";

  std::cout << "source: "
            << inbound->from_host
            << ":"
            << inbound->from_port
            << "\n";

  server.stop();

  return 0;
}
