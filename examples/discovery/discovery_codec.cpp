/*
 * discovery_codec.cpp
 */

#include <iostream>

#include <softadastra/discovery/Discovery.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== DISCOVERY CODEC EXAMPLE ==\n";

  std::vector<std::uint8_t> payload{'h', 'e', 'l', 'l', 'o'};

  auto message =
      discovery::core::DiscoveryMessage::announce(
          "node-a",
          payload);

  message.to_node_id = "node-b";
  message.correlation_id = "announce-1";

  auto encoded =
      discovery::encoding::DiscoveryEncoder::encode_message(message);

  if (encoded.empty())
  {
    std::cerr << "failed to encode discovery message\n";
    return 1;
  }

  auto decoded =
      discovery::encoding::DiscoveryDecoder::decode_message(encoded);

  if (!decoded.has_value())
  {
    std::cerr << "failed to decode discovery message\n";
    return 1;
  }

  std::cout << "encoded size: "
            << encoded.size()
            << "\n";

  std::cout << "decoded type: "
            << discovery::types::to_string(decoded->type)
            << "\n";

  std::cout << "payload size: "
            << decoded->payload_size()
            << "\n";

  return 0;
}
