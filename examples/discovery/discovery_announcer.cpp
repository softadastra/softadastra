/*
 * discovery_announcer.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY ANNOUNCER EXAMPLE ==\n";

  auto config =
      discovery::core::DiscoveryConfig::local(
          "node-announcer",
          9400,
          7000);

  discovery::backend::UdpDiscoveryBackend backend{config};

  if (!backend.start())
  {
    std::cerr << "failed to start UDP discovery backend\n";
    return 1;
  }

  discovery::core::DiscoveryAnnouncement announcement{
      config.node_id,
      config.announce_host,
      config.announce_port};

  std::vector<std::uint8_t> payload{
      announcement.node_id.begin(),
      announcement.node_id.end()};

  auto message =
      discovery::core::DiscoveryMessage::announce(
          config.node_id,
          payload);

  message.correlation_id = "announce-demo-1";

  discovery::client::DiscoveryClient client{backend};

  const bool sent =
      client.send_announce(
          config.broadcast_host,
          config.broadcast_port,
          config.node_id,
          payload);

  std::cout << "announcement sent: "
            << sent
            << "\n";

  backend.stop();

  return 0;
}
