/*
 * basic_client.cpp
 */

#include <iostream>

#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT BASIC CLIENT EXAMPLE ==\n";

  auto config =
      transport::core::TransportConfig::local(7001);

  transport::backend::TcpTransportBackend backend{config};

  if (!backend.start())
  {
    std::cerr << "failed to start local backend\n";
    return 1;
  }

  transport::client::TransportClient client{backend};

  transport::core::PeerInfo peer{
      "node-server",
      "127.0.0.1",
      7000};

  if (!client.connect(peer))
  {
    std::cerr << "failed to connect to peer\n";
    backend.stop();
    return 1;
  }

  if (!client.send_hello(peer, "node-client"))
  {
    std::cerr << "failed to send hello\n";
    backend.stop();
    return 1;
  }

  std::cout << "hello sent to "
            << peer.node_id
            << "\n";

  backend.stop();

  return 0;
}
