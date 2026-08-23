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

#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <iostream>
#include <string_view>

namespace
{

  const char *software_state_name(
      softadastra::SoftwareState state) noexcept
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
        << "Softadastra\n"
        << '\n'
        << "Usage:\n"
        << "  softadastra register <software-id>\n"
        << "  softadastra start <software-id>\n"
        << "  softadastra stop <software-id>\n"
        << "  softadastra status <software-id>\n"
        << "  softadastra connectivity\n"
        << "  softadastra help\n";
  }

} // namespace

namespace softadastra
{

  Cli::Cli(
      ControlClient &client,
      Process &process) noexcept
      : client_(client),
        process_(process)
  {
  }

  int Cli::run(
      int argc,
      const char *const argv[])
  {
    if (argc < 2)
    {
      print_usage();
      return 2;
    }

    const std::string_view command(argv[1]);

    if (command == "help" ||
        command == "--help" ||
        command == "-h")
    {
      print_usage();
      return 0;
    }

    if (command == "connectivity")
    {
      if (!client_.connectivity_available())
      {
        std::cout << "network: unavailable\n";
        return 0;
      }

      std::cout
          << "network: available\n"
          << "connected: "
          << (client_.connected() ? "yes" : "no")
          << '\n';

      return 0;
    }

    if (argc < 3)
    {
      std::cerr
          << "Missing software identifier.\n\n";

      print_usage();
      return 2;
    }

    const SoftwareId id(argv[2]);

    if (command == "register")
    {
      if (!client_.register_software(id))
      {
        std::cerr
            << "Software is already registered: "
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

    if (command == "start")
    {
      if (!client_.start_software(id, process_))
      {
        std::cerr
            << "Failed to start software: "
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
      if (!client_.stop_software(id, process_))
      {
        std::cerr
            << "Failed to stop software: "
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
      const auto state =
          client_.software_state(id);

      if (!state.has_value())
      {
        std::cerr
            << "Software is not registered: "
            << id.value()
            << '\n';

        return 1;
      }

      std::cout
          << id.value()
          << ": "
          << software_state_name(*state)
          << '\n';

      return 0;
    }

    std::cerr
        << "Unknown command: "
        << command
        << "\n\n";

    print_usage();

    return 2;
  }

} // namespace softadastra
