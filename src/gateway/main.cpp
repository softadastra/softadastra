#include "control/ControlClient.hpp"
#include "gateway/ControlLocalGatewayTargetResolver.hpp"
#include "host/HostObservation.hpp"
#include "host/LocalGateway.hpp"

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string>

#if defined(__linux__)
#include <arpa/inet.h>
#include <csignal>
#include <pthread.h>
#endif

namespace
{
  struct GatewayArguments
  {
    std::string address;
    std::uint16_t port{};
    std::filesystem::path control;
  };

  bool parse_listen(const std::string &value, GatewayArguments &arguments)
  {
#if defined(__linux__)
    const auto delimiter = value.rfind(':');
    if (delimiter == std::string::npos || delimiter == 0 || delimiter + 1 == value.size()) return false;
    const std::string address = value.substr(0, delimiter);
    const std::string port = value.substr(delimiter + 1);
    if (port.empty() || port.size() > 5) return false;
    unsigned long number = 0;
    for (const char character : port)
    {
      if (character < '0' || character > '9') return false;
      const auto digit = static_cast<unsigned long>(character - '0');
      number = number * 10UL + digit;
    }
    in_addr parsed{};
    if (number == 0 || number > 65535 || ::inet_pton(AF_INET, address.c_str(), &parsed) != 1) return false;
    arguments.address = address;
    arguments.port = static_cast<std::uint16_t>(number);
    return true;
#else
    static_cast<void>(value); static_cast<void>(arguments); return false;
#endif
  }

  bool parse_arguments(int argc, char **argv, GatewayArguments &arguments)
  {
    if (argc != 5) return false;
    bool listen = false;
    bool control = false;
    for (int index = 1; index < argc; index += 2)
    {
      const std::string option(argv[index]);
      if (option == "--listen" && !listen) listen = parse_listen(argv[index + 1], arguments);
      else if (option == "--control" && !control && argv[index + 1][0] != '\0') { arguments.control = argv[index + 1]; control = true; }
      else return false;
    }
    return listen && control;
  }
}

int main(int argc, char **argv)
{
  GatewayArguments arguments;
  if (!parse_arguments(argc, argv, arguments))
  {
    std::cerr << "usage: softadastra-gateway --listen <ipv4>:<port> --control <control-endpoint>\n";
    return 2;
  }

#if defined(__linux__)
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGINT);
  sigaddset(&signals, SIGTERM);
  if (pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0) return 1;

  softadastra::ControlClient client(arguments.control);
  if (!softadastra::observe_host(
           client,
           arguments.control.parent_path())
           .available())
  {
    std::cerr << "local control is unavailable\n";
    return 1;
  }
  softadastra::ControlLocalGatewayTargetResolver resolver(client);
  softadastra::LocalGateway gateway(resolver);
  if (!gateway.start(arguments.address, arguments.port))
  {
    std::cerr << "cannot listen on " << arguments.address << ':' << arguments.port << '\n';
    return 1;
  }
  int signal = 0;
  if (::sigwait(&signals, &signal) != 0) return 1;
  gateway.stop();
  return 0;
#else
  std::cerr << "softadastra-gateway is supported on Linux only\n";
  return 1;
#endif
}
