#include "platform/FirewallHelper.hpp"
#include "platform/NativeLocalFirewall.hpp"
#include "platform/NativeNetwork.hpp"

#include <iostream>

int main(int argc, char **argv)
{
  softadastra::NativeNetwork network;
  softadastra::NativeLocalFirewall firewall;
  softadastra::FirewallHelper helper(network, firewall);
  std::vector<std::string> arguments;
  for (int index = 1; index < argc; ++index) arguments.emplace_back(argv[index]);
  const std::string executable(argv[0]);
  const auto command = executable.find("firewall-status") != std::string::npos
                           ? softadastra::FirewallHelperCommand::Status
                           : softadastra::FirewallHelperCommand::Modify;
  return helper.run(command, arguments, std::cout);
}
