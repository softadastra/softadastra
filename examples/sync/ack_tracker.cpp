/*
 * ack_tracker.cpp
 */

#include <iostream>

#include <softadastra/sync/Sync.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== ACK TRACKER EXAMPLE ==\n";

  sync::ack::AckTracker tracker;

  tracker.track(
      "node-a-1",
      core::time::Duration::from_seconds(10));

  std::cout << "Tracked count: "
            << tracker.size()
            << "\n";

  std::cout << "Waiting: "
            << tracker.is_waiting("node-a-1")
            << "\n";

  tracker.ack("node-a-1");

  std::cout << "Acknowledged: "
            << tracker.acknowledged("node-a-1")
            << "\n";

  auto removed = tracker.prune_received();

  std::cout << "Removed: "
            << removed
            << "\n";

  return tracker.empty() ? 0 : 1;
}
