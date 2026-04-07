/*
 * basic_client.cpp
 */

#include <iostream>

#include <softadastra/transport/backend/TcpTransportBackend.hpp>
#include <softadastra/transport/client/TransportClient.hpp>
#include <softadastra/transport/core/PeerInfo.hpp>
#include <softadastra/transport/core/TransportConfig.hpp>
#include <softadastra/transport/core/TransportMessage.hpp>
#include <softadastra/transport/types/MessageType.hpp>

using namespace softadastra;

int main()
{
  transport::core::TransportConfig cfg;
  cfg.bind_host = "0.0.0.0";
  cfg.bind_port = 9002;

  transport::backend::TcpTransportBackend backend(cfg);

  if (!backend.start())
  {
    std::cerr << "Failed to start backend\n";
    return 1;
  }

  transport::client::TransportClient client(backend);

  transport::core::PeerInfo server;
  server.node_id = "node-server";
  server.host = "127.0.0.1";
  server.port = 9000;

  if (!client.connect(server))
  {
    std::cerr << "Failed to connect to server\n";
    return 1;
  }

  transport::core::TransportMessage msg;
  msg.type = transport::types::MessageType::Ping;
  msg.from_node_id = "node-client";
  msg.to_node_id = "node-server";
  msg.correlation_id = "ping-1";

  if (!client.send_to(server, msg))
  {
    std::cerr << "Send failed\n";
    return 1;
  }

  std::cout << "Ping sent\n";

  return 0;
}
