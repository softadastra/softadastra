/*
 * snapshot_from_wal.cpp
 */

#include <filesystem>
#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE SNAPSHOT FROM WAL EXAMPLE ==\n";

  const std::string wal_path = "snapshot_from_wal.wal";
  std::filesystem::remove(wal_path);

  {
    store::engine::StoreEngine engine{
        store::core::StoreConfig::durable(wal_path)};

    auto a = engine.put(
        store::types::Key{"item:1"},
        store::types::Value::from_string("alpha"));

    auto b = engine.put(
        store::types::Key{"item:2"},
        store::types::Value::from_string("beta"));

    auto c = engine.put(
        store::types::Key{"item:1"},
        store::types::Value::from_string("alpha-updated"));

    if (a.is_err() || b.is_err() || c.is_err())
    {
      std::cerr << "failed to write WAL-backed store entries\n";
      return 1;
    }
  }

  auto snapshot_result =
      store::snapshot::SnapshotBuilderStore::build(wal_path);

  if (snapshot_result.is_err())
  {
    std::cerr << "snapshot build failed: "
              << snapshot_result.error().message()
              << "\n";
    return 1;
  }

  auto snapshot = std::move(snapshot_result.value());

  std::cout << "Snapshot size: "
            << snapshot.size()
            << "\n";

  for (const auto &[key, value] : snapshot.all())
  {
    std::cout << key
              << " => "
              << value.to_string()
              << "\n";
  }

  std::filesystem::remove(wal_path);

  return 0;
}
