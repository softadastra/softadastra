#include "host/LocalGatewayProcessEndpoint.hpp"

#include <gtest/gtest.h>

#include <memory>

namespace
{
  struct ProcessState
  {
    bool running{true};
    int stops{};
  };
  class TestProcess final : public softadastra::Process
  {
  public:
    explicit TestProcess(std::shared_ptr<ProcessState> value) : value_(std::move(value)) {}
    bool stop() override
    {
      value_->running = false;
      ++value_->stops;
      return true;
    }
    [[nodiscard]] bool is_running() const noexcept override { return value_->running; }
    [[nodiscard]] std::optional<int> exit_code() noexcept override { return value_->running ? std::nullopt : std::optional<int>(0); }

  private:
    std::shared_ptr<ProcessState> value_;
  };

  class Launcher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] softadastra::ProcessLaunchResult launch(const softadastra::ProcessSpec &spec) override
    {
      last = spec;
      if (!succeeds)
        return nullptr;
      state = std::make_shared<ProcessState>();
      return std::make_unique<TestProcess>(state);
    }
    bool succeeds{true};
    softadastra::ProcessSpec last{""};
    std::shared_ptr<ProcessState> state;
  };

  TEST(LocalGatewayProcessEndpointTest, LaunchesOnlyTheDedicatedGatewayAndStopsIt)
  {
    Launcher launcher;
    softadastra::LocalGatewayProcessEndpoint endpoint(launcher, "/opt/softadastra/softadastra-gateway", "/run/softadastra/control.sock");
    EXPECT_TRUE(endpoint.start("10.42.0.1", 18080));
    EXPECT_EQ(launcher.last.executable(), "/opt/softadastra/softadastra-gateway");
    EXPECT_EQ(launcher.last.arguments(), std::vector<std::string>({"--listen", "10.42.0.1:18080", "--control", "/run/softadastra/control.sock"}));
    EXPECT_TRUE(endpoint.start("10.42.0.1", 18080));
    endpoint.stop();
    EXPECT_EQ(launcher.state->stops, 1);
    endpoint.stop();
    EXPECT_EQ(launcher.state->stops, 1);
  }

  TEST(LocalGatewayProcessEndpointTest, ReportsLaunchFailure)
  {
    Launcher launcher;
    launcher.succeeds = false;
    softadastra::LocalGatewayProcessEndpoint endpoint(launcher, "gateway", "control.sock");
    EXPECT_FALSE(endpoint.start("10.42.0.1", 18080));
  }
}
