/*
 * metadata_minimal.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA MINIMAL EXAMPLE ==\n";

  auto options =
      metadata::MetadataOptions::local(
          "node-a",
          "0.1.0");

  if (!options.is_valid())
  {
    std::cerr << "invalid metadata options\n";
    return 1;
  }

  auto config =
      options.to_config();

  std::cout << "node id: "
            << config.node_id
            << "\n";

  std::cout << "display name: "
            << config.display_name
            << "\n";

  std::cout << "version: "
            << config.version
            << "\n";

  std::cout << "refresh interval ms: "
            << config.refresh_interval_ms()
            << "\n";

  return 0;
}
