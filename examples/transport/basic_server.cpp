/*
 * basic_server.cpp
 */

#include <iostream>

#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT BASIC SERVER EXAMPLE ==\n";

  auto config =
      transport::core::TransportConfig::local(7000);

  transport::backend::TcpTransportBackend backend{config};
  transport::server::TransportServer server{backend};

  if (!server.start())
  {
    std::cerr << "failed to start server\n";
    return 1;
  }

  std::cout << "server running on "
            << config.bind_host
            << ":"
            << config.bind_port
            << "\n";

  std::cout << "polling once...\n";

  auto inbound = server.poll();

  if (inbound.has_value())
  {
    std::cout << "received message from: "
              << inbound->message.from_node_id
              << "\n";

    std::cout << "message type: "
              << transport::types::to_string(inbound->message.type)
              << "\n";
  }
  else
  {
    std::cout << "no inbound message available\n";
  }

  server.stop();

  return 0;
}
