/**
 *
 *  @file main.cpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2026, Gaspard Kirira.
 *  https://github.com/softadastra/softadastra
 *
 *  Licensed under the Apache License, Version 2.0.
 *  See the LICENSE file in the project root for license information.
 *
 *  Softadastra
 */

#include "cli/Cli.hpp"
#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "control/LocalControlServer.hpp"
#include "control/RemoteAccessConfig.hpp"
#include "control/RemoteControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostIdentity.hpp"
#include "host/HostLoop.hpp"
#include "host/HostService.hpp"
#include "host/HostStateFile.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "platform/HostInstanceLock.hpp"
#include "platform/MdnsPublisher.hpp"
#include "platform/NativePlatform.hpp"

#include <chrono>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if defined(__linux__)

#include <csignal>
#include <pthread.h>

#endif

namespace
{
#if defined(__linux__)
  bool block_host_signals()
  {
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    return pthread_sigmask(SIG_BLOCK, &signals, nullptr) == 0;
  }
#endif

  void print_host_started(
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings,
      const softadastra::MdnsPublisher &mdns_publisher,
      bool mdns_available)
  {
    const auto access = host_service.local_access();
    const std::string ipv4 = host_service.primary_ipv4();

    std::cout << "Softadastra Host is running\n"
              << "hostname: "
              << (access.host_name.empty() ? "unavailable" : access.host_name)
              << "\nlocal control: ready\n"
              << "network: "
              << (host_service.connectivity_available() ? "available" : "unavailable")
              << "\nipv4: " << (ipv4.empty() ? "unavailable" : ipv4)
              << "\nlocal name: "
              << (mdns_available ? mdns_publisher.name()
                                 : "unavailable (mDNS unavailable)")
              << "\nremote access: "
              << (remote_settings.enabled ? "enabled" : "disabled")
              << "\n\nPress Ctrl+C to stop.\n"
              << std::flush;
  }

  int run_host(
      softadastra::HostLoop &host_loop,
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings,
      softadastra::MdnsPublisher &mdns_publisher)
  {
#if defined(__linux__)

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    if (pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0)
    {
      return 1;
    }

    std::atomic_bool completed{false};
    std::thread thread([&host_loop, &completed]()
                       {
                         completed = host_loop.run();
                       });

    for (int attempt = 0; attempt < 1000; ++attempt)
    {
      if (host_loop.is_running())
      {
        const bool mdns_available =
            mdns_publisher.start(host_service.primary_ipv4());
        print_host_started(
            host_service,
            remote_settings,
            mdns_publisher,
            mdns_available);
        break;
      }

      if (completed)
      {
        thread.join();
        return 1;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (!host_loop.is_running())
    {
      host_loop.request_stop();
      thread.join();
      return 1;
    }

    int signal = 0;
    const int result = sigwait(&signals, &signal);

    if (result == 0)
    {
      std::cout << "Stopping Softadastra Host...\n" << std::flush;
    }

    host_loop.request_stop();
    thread.join();
    mdns_publisher.stop();

    if (result == 0 && completed)
    {
      std::cout << "Softadastra Host stopped.\n" << std::flush;
      return 0;
    }

    return 1;

#else

    const bool completed = host_loop.run();
    return completed ? 0 : 1;

#endif
  }
} // namespace

int main(int argc, char *argv[])
{
  if (!softadastra::NativePlatform::host_supported())
  {
    std::cerr << softadastra::NativePlatform::host_support_diagnostic()
              << '\n';
    return 1;
  }

  if (argc == 2 && std::string(argv[1]) == "host")
  {
#if defined(__linux__)
    if (!block_host_signals())
    {
      return 1;
    }
#endif

    const auto data_directory = softadastra::NativeDataDirectory::path();

    if (!softadastra::NativeDataDirectory::ensure_exists())
    {
      std::cerr << "failed to initialize Host data directory\n";
      return 1;
    }

    softadastra::HostInstanceLock instance_lock;

    if (!instance_lock.acquire(data_directory))
    {
      std::cerr << "Softadastra Host is already running\n";
      return 1;
    }

    softadastra::HostIdentity identity(
        data_directory / "identity");

    if (!identity.load_or_create())
    {
      std::cerr << "failed to load Host identity\n";
      return 1;
    }

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService host_service(
        host,
        platform.process_launcher());
    softadastra::HostStateFile state_file(
        data_directory / "host-state");
    softadastra::ControlServer control_server(host_service);
    softadastra::RemoteAccessConfig remote_config(
        data_directory / "remote-access");
    softadastra::RemoteAccessSettings remote_settings;
    if (!remote_config.load(remote_settings) && !remote_config.save(remote_settings))
    {
      std::cerr << "failed to initialize remote access configuration\n";
      return 1;
    }
    softadastra::RemoteControlServer remote_server(
        control_server, remote_config, identity.secret(),
        data_directory);
    if (!remote_server.apply())
    {
      std::cerr << "failed to apply remote access configuration\n";
      return 1;
    }
    softadastra::LocalControlServer local_control_server(
        control_server,
        data_directory / "control.sock",
        &remote_config, &remote_server);
    softadastra::HostLoop host_loop(
        host_service,
        state_file,
        std::chrono::seconds(1),
        &local_control_server);
    softadastra::MdnsPublisher mdns_publisher(identity.id());
    return run_host(
        host_loop,
        host_service,
        remote_settings,
        mdns_publisher);
  }

  softadastra::ControlClient control_client(
      softadastra::NativeDataDirectory::path() / "control.sock");

  softadastra::Cli cli(control_client);

  std::vector<const char *> arguments;
  arguments.reserve(static_cast<std::size_t>(argc));

  for (int index = 0; index < argc; ++index)
  {
    arguments.push_back(argv[index]);
  }

  return cli.run(
      argc,
      arguments.data());
}
