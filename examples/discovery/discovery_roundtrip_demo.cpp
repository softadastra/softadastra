/*
 * discovery_roundtrip_demo.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY ROUNDTRIP DEMO ==\n";

  auto message =
      discovery::core::DiscoveryMessage::probe("node-a");

  message.to_node_id = "node-b";
  message.correlation_id = "probe-1";

  auto datagram =
      discovery::encoding::DiscoveryEncoder::make_datagram(
          message,
          "127.0.0.1",
          9400);

  if (!datagram.is_valid())
  {
    std::cerr << "failed to encode datagram\n";
    return 1;
  }

  auto decoded =
      discovery::encoding::DiscoveryDecoder::decode_datagram(datagram);

  if (!decoded.has_value())
  {
    std::cerr << "failed to decode datagram\n";
    return 1;
  }

  std::cout << "decoded type: "
            << discovery::types::to_string(decoded->type)
            << "\n";

  std::cout << "from: "
            << decoded->from_node_id
            << "\n";

  std::cout << "to: "
            << decoded->to_node_id
            << "\n";

  std::cout << "correlation: "
            << decoded->correlation_id
            << "\n";

  return 0;
}
