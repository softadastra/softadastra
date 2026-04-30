/*
 * metadata_encode_decode_demo.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA ENCODE DECODE DEMO ==\n";

  auto original =
      metadata::core::NodeMetadata::foundation(
          "node-a",
          metadata::utils::Hostname::get(),
          metadata::utils::PlatformInfo::os_name(),
          "1.0.0");

  if (!original.is_valid())
  {
    std::cerr << "invalid original metadata\n";
    return 1;
  }

  auto encoded =
      metadata::encoding::MetadataEncoder::encode(original);

  if (encoded.empty())
  {
    std::cerr << "failed to encode metadata\n";
    return 1;
  }

  auto decoded =
      metadata::encoding::MetadataDecoder::decode(encoded);

  if (!decoded.has_value())
  {
    std::cerr << "failed to decode metadata\n";
    return 1;
  }

  std::cout << "encoded size: "
            << encoded.size()
            << "\n";

  std::cout << "decoded node id: "
            << decoded->node_id()
            << "\n";

  std::cout << "decoded hostname: "
            << decoded->runtime.hostname
            << "\n";

  std::cout << "decoded capabilities: "
            << decoded->capabilities.size()
            << "\n";

  return 0;
}
