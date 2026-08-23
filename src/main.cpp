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
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"

#include <vector>

int main(int argc, char *argv[])
{
  softadastra::NativePlatform platform;

  softadastra::Host host(platform);

  softadastra::HostService host_service(
      host,
      platform.process_launcher());

  softadastra::ControlServer control_server(host_service);
  softadastra::ControlClient control_client(control_server);

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
