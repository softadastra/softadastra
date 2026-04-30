/*
 * metadata_local_snapshot.cpp
 */

#include <iostream>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== METADATA LOCAL SNAPSHOT EXAMPLE ==\n";

  auto metadata_snapshot =
      metadata::core::NodeMetadata::foundation(
          "node-a",
          metadata::utils::Hostname::get(),
          metadata::utils::PlatformInfo::os_name(),
          metadata::utils::VersionInfo::current());

  if (!metadata_snapshot.is_valid())
  {
    std::cerr << "invalid metadata snapshot\n";
    return 1;
  }

  metadata_snapshot.refresh_runtime();

  std::cout << "node id: "
            << metadata_snapshot.node_id()
            << "\n";

  std::cout << "label: "
            << metadata_snapshot.label()
            << "\n";

  std::cout << "hostname: "
            << metadata_snapshot.runtime.hostname
            << "\n";

  std::cout << "os: "
            << metadata_snapshot.runtime.os_name
            << "\n";

  std::cout << "version: "
            << metadata_snapshot.runtime.version
            << "\n";

  std::cout << "uptime ms: "
            << metadata_snapshot.runtime.uptime_ms()
            << "\n";

  std::cout << "capabilities: "
            << metadata_snapshot.capabilities.size()
            << "\n";

  return 0;
}
