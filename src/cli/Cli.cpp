/**
 *
 *  @file Cli.cpp
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

#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace
{
  const char *software_state_name(softadastra::SoftwareState state) noexcept
  {
    switch (state)
    {
    case softadastra::SoftwareState::Stopped:
      return "stopped";

    case softadastra::SoftwareState::Starting:
      return "starting";

    case softadastra::SoftwareState::Running:
      return "running";

    case softadastra::SoftwareState::Failed:
      return "failed";
    }

    return "unknown";
  }

  void print_usage()
  {
    std::cout
        << "Usage:\n"
        << "  softadastra register <software-id> <executable> [arguments...]\n"
        << "  softadastra start <software-id>\n"
        << "  softadastra stop <software-id>\n"
        << "  softadastra status <software-id>\n"
        << "  softadastra connectivity\n"
        << "  softadastra help\n";
  }

} // namespace

namespace softadastra
{

  Cli::Cli(ControlClient &client) noexcept
      : client_(client)
  {
  }

  int Cli::run(int argc, const char *const argv[])
  {
    if (argc < 2)
    {
      print_usage();
      return 2;
    }

    const std::string command(argv[1]);

    if (command == "help" ||
        command == "--help" ||
        command == "-h")
    {
      print_usage();
      return 0;
    }

    if (command == "connectivity")
    {
      if (argc != 2)
      {
        std::cerr << "connectivity does not accept arguments\n";
        return 2;
      }

      if (!client_.connectivity_available())
      {
        std::cout << "network: unavailable\n";
        return 0;
      }

      std::cout << "network: available\n"
                << "connected: "
                << (client_.connected() ? "yes" : "no")
                << '\n';

      return 0;
    }

    if (command == "register")
    {
      if (argc < 4)
      {
        std::cerr
            << "register requires a software identifier and an executable\n";
        return 2;
      }

      std::vector<std::string> arguments;
      arguments.reserve(static_cast<std::size_t>(argc - 4));

      for (int index = 4; index < argc; ++index)
      {
        arguments.emplace_back(argv[index]);
      }

      const SoftwareId id(argv[2]);
      const ProcessSpec process_spec(
          argv[3],
          std::move(arguments));

      if (!client_.register_software(id, process_spec))
      {
        std::cerr
            << "failed to register software: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << "registered: "
          << id.value()
          << '\n';

      return 0;
    }

    if (argc != 3)
    {
      std::cerr
          << command
          << " requires a software identifier\n";

      return 2;
    }

    const SoftwareId id(argv[2]);

    if (command == "start")
    {
      if (!client_.start_software(id))
      {
        std::cerr
            << "failed to start software: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << "started: "
          << id.value()
          << '\n';

      return 0;
    }

    if (command == "stop")
    {
      if (!client_.stop_software(id))
      {
        std::cerr
            << "failed to stop software: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << "stopped: "
          << id.value()
          << '\n';

      return 0;
    }

    if (command == "status")
    {
      const auto state = client_.software_state(id);

      if (!state.has_value())
      {
        std::cerr
            << "software not found: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << id.value()
          << ": "
          << software_state_name(state.value())
          << '\n';

      return 0;
    }

    std::cerr
        << "unknown command: "
        << command
        << '\n';

    print_usage();

    return 2;
  }

} // namespace softadastra
