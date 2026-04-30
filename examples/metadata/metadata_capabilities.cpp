/*
 * metadata_capabilities.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA CAPABILITIES EXAMPLE ==\n";

  metadata::core::NodeCapabilities capabilities;

  capabilities.add(metadata::types::CapabilityType::Core);
  capabilities.add(metadata::types::CapabilityType::Store);
  capabilities.add(metadata::types::CapabilityType::Sync);
  capabilities.add(metadata::types::CapabilityType::Transport);
  capabilities.add(metadata::types::CapabilityType::Metadata);

  std::cout << "capability count: "
            << capabilities.size()
            << "\n";

  std::cout << "has sync: "
            << capabilities.has(metadata::types::CapabilityType::Sync)
            << "\n";

  std::cout << "has foundation capability: "
            << capabilities.has_foundation_capability()
            << "\n";

  std::cout << "has user-facing capability: "
            << capabilities.has_user_facing_capability()
            << "\n";

  for (const auto capability : capabilities.values)
  {
    std::cout << "- "
              << metadata::types::to_string(capability)
              << "\n";
  }

  return 0;
}
