/*
 * message_codec.cpp
 */

#include <iostream>

#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT MESSAGE CODEC EXAMPLE ==\n";

  auto message =
      transport::core::TransportMessage::ping("node-a");

  message.to_node_id = "node-b";
  message.correlation_id = "ping-1";

  auto frame =
      transport::encoding::MessageEncoder::encode_frame(message);

  if (frame.empty())
  {
    std::cerr << "failed to encode message\n";
    return 1;
  }

  auto decoded =
      transport::encoding::MessageDecoder::decode_framed_message(frame);

  if (!decoded.has_value())
  {
    std::cerr << "failed to decode framed message\n";
    return 1;
  }

  std::cout << "decoded type: "
            << transport::types::to_string(decoded->type)
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
