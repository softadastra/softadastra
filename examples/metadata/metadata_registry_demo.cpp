/*
 * metadata_registry_demo.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA REGISTRY DEMO ==\n";

  metadata::registry::MetadataRegistry registry;

  auto node_a =
      metadata::core::NodeMetadata::foundation(
          "node-a",
          "host-a",
          "linux",
          "1.0.0");

  auto node_b =
      metadata::core::NodeMetadata::minimal(
          "node-b",
          "host-b",
          "linux",
          "1.0.0");

  registry.upsert(node_a);
  registry.upsert(node_b);

  std::cout << "registry size: "
            << registry.size()
            << "\n";

  auto found =
      registry.get("node-a");

  if (found.has_value())
  {
    std::cout << "found node: "
              << found->node_id()
              << "\n";
  }

  registry.erase("node-b");

  std::cout << "registry size after erase: "
            << registry.size()
            << "\n";

  return 0;
}
