/*
 * basic_server.cpp
 */

#include <chrono>
#include <cstddef>
#include <iostream>
#include <thread>

#include <vix/async/core/io_context.hpp>

#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== TRANSPORT BASIC ASYNC SERVER EXAMPLE ==\n";

  vix::async::core::io_context context{};

  auto config =
      transport::core::TransportConfig::local(7000);

  config.enable_keepalive = false;

  transport::backend::AsyncTcpTransportBackend backend{
      context,
      config};

  if (!backend.start())
  {
    std::cerr << "failed to start async server backend\n";
    return 1;
  }

  std::thread runner(
      [&context]()
      {
        context.run();
      });

  std::cout << "server running on "
            << config.bind_host
            << ":"
            << config.bind_port
            << "\n";

  std::cout << "waiting for transport events...\n";

  bool received = false;

  for (std::size_t tick = 0; tick < 50; ++tick)
  {
    std::this_thread::sleep_for(
        std::chrono::milliseconds{100});

    while (true)
    {
      auto event = backend.poll_event();

      if (!event.has_value())
      {
        break;
      }

      if (event->type ==
          transport::core::TransportEventType::PeerConnected)
      {
        if (event->peer.has_value())
        {
          std::cout << "peer connected: "
                    << event->peer->node_id
                    << "\n";
        }
      }

      if (event->type ==
              transport::core::TransportEventType::EnvelopeReceived &&
          event->envelope.has_value())
      {
        const auto &inbound = *event->envelope;

        std::cout << "received message from: "
                  << inbound.message.from_node_id
                  << "\n";

        std::cout << "message type: "
                  << transport::types::to_string(
                         inbound.message.type)
                  << "\n";

        received = true;
      }

      if (event->type ==
          transport::core::TransportEventType::BackendError)
      {
        std::cerr << "backend error: "
                  << event->message
                  << "\n";
      }
    }

    if (received)
    {
      break;
    }
  }

  if (!received)
  {
    std::cout << "no inbound message received\n";
  }

  backend.stop();
  context.stop();

  if (runner.joinable())
  {
    runner.join();
  }

  return received ? 0 : 1;
}
