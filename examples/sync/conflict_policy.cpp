/*
 * conflict_policy.cpp
 */

#include <iostream>

#include <softadastra/store/Store.hpp>
#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== CONFLICT POLICY EXAMPLE ==\n";

  auto local_entry = store::core::Entry::make(
      store::types::Key{"doc:1"},
      store::types::Value::from_string("local"),
      1);

  auto remote_operation = store::core::Operation::put(
      store::types::Key{"doc:1"},
      store::types::Value::from_string("remote"));

  auto resolution =
      sync::conflict::ConflictResolver::resolve(
          local_entry,
          remote_operation,
          sync::types::ConflictPolicy::LastWriteWins,
          "node-a",
          "node-b");

  std::cout << "Conflict detected: "
            << resolution.conflict_detected
            << "\n";

  std::cout << "Apply remote: "
            << resolution.apply_remote
            << "\n";

  std::cout << "Keep local: "
            << resolution.keep_local
            << "\n";

  return resolution.is_valid() ? 0 : 1;
}
