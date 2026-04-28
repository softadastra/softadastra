/*
 * main.cpp
 */

#include <csignal>
#include <iostream>

#include "SoftadastraNode.hpp"

namespace
{
  volatile std::sig_atomic_t g_stop_requested = 0;

  void handle_signal(int) noexcept
  {
    g_stop_requested = 1;
  }
}

int main()
{
  std::signal(SIGINT, handle_signal);
  std::signal(SIGTERM, handle_signal);

  softadastra::app::node::SoftadastraNode node;

  if (!node.valid())
  {
    std::cerr << "Failed to initialize Softadastra node.\n";
    return 1;
  }

  if (!node.start())
  {
    std::cerr << "Failed to start Softadastra node.\n";
    return 1;
  }

  std::cout << "Softadastra node running.\n";
  std::cout << "Press Ctrl+C to stop.\n";

  while (g_stop_requested == 0)
  {
    node.tick();
  }

  node.stop();

  std::cout << "\nSoftadastra node stopped.\n";

  return 0;
}
