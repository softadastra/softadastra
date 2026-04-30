/*
 * discovery_minimal.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY MINIMAL EXAMPLE ==\n";

  auto options =
      discovery::DiscoveryOptions::local(
          "node-a",
          9400,
          7000);

  if (!options.is_valid())
  {
    std::cerr << "invalid discovery options\n";
    return 1;
  }

  auto config =
      options.to_config();

  std::cout << "node id: "
            << config.node_id
            << "\n";

  std::cout << "bind: "
            << config.bind_host
            << ":"
            << config.bind_port
            << "\n";

  std::cout << "announce: "
            << config.announce_host
            << ":"
            << config.announce_port
            << "\n";

  std::cout << "announce interval ms: "
            << config.announce_interval_ms()
            << "\n";

  std::cout << "peer ttl ms: "
            << config.peer_ttl_ms()
            << "\n";

  return 0;
}
