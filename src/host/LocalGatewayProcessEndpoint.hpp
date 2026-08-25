#ifndef SOFTADASTRA_HOST_LOCAL_GATEWAY_PROCESS_ENDPOINT_HPP
#define SOFTADASTRA_HOST_LOCAL_GATEWAY_PROCESS_ENDPOINT_HPP

#include "host/LocalReachability.hpp"
#include "platform/ProcessLauncher.hpp"

#include <filesystem>
#include <memory>

namespace softadastra
{
  class LocalGatewayProcessEndpoint final : public LocalGatewayEndpoint
  {
  public:
    LocalGatewayProcessEndpoint(ProcessLauncher &, std::filesystem::path executable, std::filesystem::path control) noexcept;
    bool start(std::string address, std::uint16_t port) override;
    void stop() noexcept override;

  private:
    ProcessLauncher &launcher_;
    std::filesystem::path executable_;
    std::filesystem::path control_;
    std::unique_ptr<Process> process_;
  };
}

#endif
