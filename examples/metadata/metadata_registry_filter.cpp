/*
 * metadata_registry_filter.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA REGISTRY FILTER EXAMPLE ==\n";

  metadata::registry::MetadataRegistry registry;

  auto foundation_node =
      metadata::core::NodeMetadata::foundation(
          "node-foundation",
          "foundation-host",
          "linux",
          "1.0.0");

  auto app_node =
      metadata::core::NodeMetadata::minimal(
          "node-app",
          "app-host",
          "linux",
          "1.0.0");

  app_node.capabilities.add(
      metadata::types::CapabilityType::App);

  registry.upsert(foundation_node);
  registry.upsert(app_node);

  const auto sync_nodes =
      registry.with_capability(
          metadata::types::CapabilityType::Sync);

  const auto foundation_nodes =
      registry.foundation_nodes();

  const auto user_facing_nodes =
      registry.user_facing_nodes();

  std::cout << "registry size: "
            << registry.size()
            << "\n";

  std::cout << "sync nodes: "
            << sync_nodes.size()
            << "\n";

  std::cout << "foundation nodes: "
            << foundation_nodes.size()
            << "\n";

  std::cout << "user-facing nodes: "
            << user_facing_nodes.size()
            << "\n";

  return 0;
}
