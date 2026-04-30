/*
 * discovery_registry.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY REGISTRY EXAMPLE ==\n";

  discovery::peer::DiscoveryRegistry registry;

  discovery::core::DiscoveryAnnouncement announcement{
      "node-b",
      "127.0.0.1",
      7001};

  registry.upsert_announcement(announcement);

  std::cout << "registry size: "
            << registry.size()
            << "\n";

  std::cout << "available peers: "
            << registry.available_peers().size()
            << "\n";

  registry.mark_stale("node-b");

  std::cout << "stale peers: "
            << registry.stale_peers().size()
            << "\n";

  registry.mark_expired("node-b");

  std::cout << "expired peers: "
            << registry.expired_peers().size()
            << "\n";

  const auto pruned =
      registry.prune_expired();

  std::cout << "pruned peers: "
            << pruned
            << "\n";

  return 0;
}
