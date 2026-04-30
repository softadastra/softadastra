/*
 * peer_registry.cpp
 */

#include <iostream>

#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT PEER REGISTRY EXAMPLE ==\n";

  transport::peer::PeerRegistry registry;

  transport::core::PeerInfo peer{
      "node-b",
      "127.0.0.1",
      7000};

  registry.upsert_peer(peer);

  registry.mark_connected("node-b");

  std::cout << "registry size: "
            << registry.size()
            << "\n";

  std::cout << "connected peers: "
            << registry.connected_peers().size()
            << "\n";

  registry.mark_faulted("node-b");

  std::cout << "faulted peers: "
            << registry.faulted_peers().size()
            << "\n";

  return 0;
}
