/**
 *
 *  @file FirewallHelperMain.cpp
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

#include "platform/FirewallHelper.hpp"
#include "platform/NativeLocalFirewall.hpp"
#include "platform/NativeNetwork.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(
    int argc,
    char **argv)
{
  softadastra::NativeNetwork network;
  softadastra::NativeLocalFirewall firewall;

  softadastra::FirewallHelper helper(
      network,
      firewall);

  std::vector<std::string> arguments;

  for (int index = 1;
       index < argc;
       ++index)
  {
    arguments.emplace_back(
        argv[index]);
  }

  const std::string executable(
      argv[0]);

  const auto command =
      executable.find("firewall-status") != std::string::npos
          ? softadastra::FirewallHelperCommand::Status
          : softadastra::FirewallHelperCommand::Modify;

  return helper.run(
      command,
      arguments,
      std::cout);
}
