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
#include "host/Host.hpp"
#include "host/HostLoop.hpp"
#include "host/HostService.hpp"
#include "host/HostStateFile.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "platform/NativePlatform.hpp"

#include <chrono>
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
  int run_host(softadastra::HostLoop &host_loop)
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

    bool completed = false;
    std::thread thread([&host_loop, &completed]()
                       {
                         completed = host_loop.run();
                       });
    int signal = 0;
    const int result = sigwait(&signals, &signal);
    host_loop.request_stop();
    thread.join();
    return result == 0 && completed ? 0 : 1;

#else

    return host_loop.run() ? 0 : 1;

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
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService host_service(
        host,
        platform.process_launcher());
    softadastra::HostStateFile state_file(
        softadastra::NativeDataDirectory::path() / "host-state");
    softadastra::ControlServer control_server(host_service);
    softadastra::LocalControlServer local_control_server(
        control_server,
        softadastra::NativeDataDirectory::path() / "control.sock");
    softadastra::HostLoop host_loop(
        host_service,
        state_file,
        std::chrono::seconds(1),
        &local_control_server);
    return run_host(host_loop);
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
