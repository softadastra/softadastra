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
  void print_host_started(
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings)
  {
    const auto access = host_service.local_access();
    std::string ipv4 = "unavailable";

    for (const auto &address : access.addresses)
    {
      if (address.family == softadastra::LocalAddressFamily::IPv4)
      {
        ipv4 = address.value;
        break;
      }
    }

    std::cout << "Softadastra Host is running\n"
              << "hostname: "
              << (access.host_name.empty() ? "unavailable" : access.host_name)
              << "\nlocal control: ready\n"
              << "network: "
              << (host_service.connectivity_available() ? "available" : "unavailable")
              << "\nipv4: " << ipv4
              << "\nremote access: "
              << (remote_settings.enabled ? "enabled" : "disabled")
              << "\n\nPress Ctrl+C to stop.\n"
              << std::flush;
  }

  int run_host(
      softadastra::HostLoop &host_loop,
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings)
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
        print_host_started(host_service, remote_settings);
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
    softadastra::HostIdentity identity(
        softadastra::NativeDataDirectory::path() / "identity");

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
        softadastra::NativeDataDirectory::path() / "host-state");
    softadastra::ControlServer control_server(host_service);
    softadastra::RemoteAccessConfig remote_config(
        softadastra::NativeDataDirectory::path() / "remote-access");
    softadastra::RemoteAccessSettings remote_settings;
    if (!remote_config.load(remote_settings) && !remote_config.save(remote_settings))
    {
      std::cerr << "failed to initialize remote access configuration\n";
      return 1;
    }
    softadastra::RemoteControlServer remote_server(
        control_server, remote_config, identity.secret(),
        softadastra::NativeDataDirectory::path());
    if (!remote_server.apply())
    {
      std::cerr << "failed to apply remote access configuration\n";
      return 1;
    }
    softadastra::LocalControlServer local_control_server(
        control_server,
        softadastra::NativeDataDirectory::path() / "control.sock",
        &remote_config, &remote_server);
    softadastra::HostLoop host_loop(
        host_service,
        state_file,
        std::chrono::seconds(1),
        &local_control_server);
    return run_host(host_loop, host_service, remote_settings);
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
