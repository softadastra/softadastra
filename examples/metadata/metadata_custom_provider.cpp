/*
 * metadata_custom_provider.cpp
 */

#include <iostream>
#include <optional>

#include <softadastra/metadata/Metadata.hpp>

using namespace softadastra;

class DemoMetadataProvider final
    : public metadata::backend::IMetadataProvider
{
public:
  std::optional<metadata::core::NodeMetadata>
  local_metadata() const override
  {
    return metadata_;
  }

  std::optional<metadata::core::NodeMetadata>
  refresh_local_metadata() override
  {
    metadata_ =
        metadata::core::NodeMetadata::foundation(
            "provider-node",
            "provider-host",
            metadata::utils::PlatformInfo::os_name(),
            "2.0.0");

    metadata_->refresh_runtime();

    return metadata_;
  }

private:
  std::optional<metadata::core::NodeMetadata> metadata_{};
};

int main()
{
  std::cout << "== METADATA CUSTOM PROVIDER EXAMPLE ==\n";

  DemoMetadataProvider provider;

  auto metadata_snapshot =
      provider.refresh_local_metadata();

  if (!metadata_snapshot.has_value())
  {
    std::cerr << "provider failed to produce metadata\n";
    return 1;
  }

  std::cout << "provider node id: "
            << metadata_snapshot->node_id()
            << "\n";

  std::cout << "provider hostname: "
            << metadata_snapshot->runtime.hostname
            << "\n";

  std::cout << "provider version: "
            << metadata_snapshot->runtime.version
            << "\n";

  return 0;
}
