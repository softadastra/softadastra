/*
 * basic_client.cpp
 */

#include <chrono>
#include <iostream>
#include <thread>

#include <vix/async/core/io_context.hpp>
#include <softadastra/transport/Transport.hpp>

using namespace softadastra;

int main()
{
  vix::async::core::io_context context{};

  auto config = transport::core::TransportConfig::local(7001);

  config.enable_keepalive = false;
  transport::backend::AsyncTcpTransportBackend backend{context, config};

  if (!backend.start())
  {
    std::cerr << "failed to start async local backend\n";
    return 1;
  }

  std::thread runner(
      [&context]()
      {
        context.run();
      });

  transport::core::PeerInfo peer{"node-server", "127.0.0.1", 7000};

  if (!backend.connect(peer))
  {
    std::cerr << "failed to connect to peer\n";

    backend.stop();
    context.stop();

    if (runner.joinable())
    {
      runner.join();
    }

    return 1;
  }

  std::this_thread::sleep_for(
      std::chrono::milliseconds{300});

  auto message =
      transport::core::TransportMessage::hello("node-client");

  message.to_node_id = peer.node_id;
  message.correlation_id = "hello-basic-client";

  transport::core::TransportEnvelope envelope{
      std::move(message),
      {},
      peer};

  if (!backend.send(envelope))
  {
    std::cerr << "failed to send hello\n";

    backend.stop();
    context.stop();

    if (runner.joinable())
    {
      runner.join();
    }

    return 1;
  }

  std::this_thread::sleep_for(
      std::chrono::milliseconds{300});

  bool sent = false;

  for (std::size_t i = 0; i < 32; ++i)
  {
    auto event = backend.poll_event();

    if (!event.has_value())
    {
      break;
    }

    if (event->type ==
        transport::core::TransportEventType::SendCompleted)
    {
      sent = true;
    }

    if (event->type ==
        transport::core::TransportEventType::SendFailed)
    {
      std::cerr << "send failed: "
                << event->message
                << "\n";
    }

    if (event->type ==
        transport::core::TransportEventType::BackendError)
    {
      std::cerr << "backend error: "
                << event->message
                << "\n";
    }
  }

  if (!sent)
  {
    std::cerr << "hello was accepted but no send completion was observed\n";
  }
  else
  {
    std::cout << "hello sent to "
              << peer.node_id
              << "\n";
  }

  backend.stop();
  context.stop();

  if (runner.joinable())
  {
    runner.join();
  }

  return sent ? 0 : 1;
}
