#include "cli/Cli.hpp"
#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/Platform.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/Service.hpp"
#include "software/ProjectIdentity.hpp"
#include "software/ProjectConfig.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>

namespace
{
  class Process final : public softadastra::Process
  {
  public:
    explicit Process(bool running = true) : running_(running) {}
    bool stop() override { running_ = false; return true; }
    [[nodiscard]] bool is_running() const noexcept override { return running_; }
    [[nodiscard]] std::optional<int> exit_code() noexcept override { return std::nullopt; }
  private: bool running_{true};
  };

  class Launcher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] softadastra::ProcessLaunchResult launch(const softadastra::ProcessSpec &spec) override
    { last_spec = spec; return std::make_unique<Process>(process_running); }
    std::optional<softadastra::ProcessSpec> last_spec;
    bool process_running{true};
  };

  class Service final : public softadastra::Service
  { public: bool start() override { return true; } bool stop() override { return true; } [[nodiscard]] bool is_running() const noexcept override { return true; } };

  class Network final : public softadastra::Network
  {
  public:
    [[nodiscard]] bool is_available() const noexcept override { return false; }
    [[nodiscard]] bool is_connected() const noexcept override { return false; }
    [[nodiscard]] std::string host_name() const override { return "host"; }
    [[nodiscard]] std::vector<softadastra::LocalNetworkAddress> local_addresses() const override { return {{softadastra::LocalAddressFamily::IPv4, "test", "127.0.0.1"}}; }
    [[nodiscard]] softadastra::NetworkCapability network_capability() const override
    {
      return capability;
    }
    softadastra::NetworkCapability capability{
        softadastra::NetworkState::Available, "10.56.116.55", "wlp108s0",
        softadastra::NetworkInterfaceType::Wifi,
        softadastra::LocalNetworkState::Existing,
        softadastra::ManagedNetworkCapability::Available};
  };

  class ManagedNetwork final : public softadastra::ManagedNetwork
  {
  public:
    [[nodiscard]] softadastra::ManagedNetworkStatus status() const override
    {
      return current;
    }
    [[nodiscard]] softadastra::ManagedNetworkStartResult start() override
    {
      ++start_calls;
      if (start_result == softadastra::ManagedNetworkStartResult::Started)
      {
        current.state = softadastra::ManagedNetworkState::Running;
      }
      return start_result;
    }
    bool stop() override { return false; }

    softadastra::ManagedNetworkStatus current{
        softadastra::ManagedNetworkCapability::Unavailable,
        softadastra::ManagedNetworkState::Stopped, {}, {}, {}};
    softadastra::ManagedNetworkStartResult start_result{
        softadastra::ManagedNetworkStartResult::Unavailable};
    int start_calls{0};
  };

  class Platform final : public softadastra::Platform
  {
  public:
    [[nodiscard]] softadastra::ProcessLauncher &process_launcher() noexcept override { return launcher; }
    [[nodiscard]] const softadastra::ProcessLauncher &process_launcher() const noexcept override { return launcher; }
    [[nodiscard]] softadastra::Service &service() noexcept override { return service_; }
    [[nodiscard]] const softadastra::Service &service() const noexcept override { return service_; }
    [[nodiscard]] softadastra::Network &network() noexcept override { return network_; }
    [[nodiscard]] const softadastra::Network &network() const noexcept override { return network_; }
    [[nodiscard]] softadastra::ManagedNetwork &managed_network() noexcept override { return managed_network_; }
    [[nodiscard]] const softadastra::ManagedNetwork &managed_network() const noexcept override { return managed_network_; }
    Launcher launcher; Service service_; Network network_; ManagedNetwork managed_network_;
  };

  TEST(CliTest, RunsMovedProjectFromRootAndSubdirectoryUsingCurrentRoot)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-cli-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto old_root = base / "old";
    const auto new_root = base / "new";
    std::filesystem::create_directories(old_root / "src");
    const auto identity = softadastra::ProjectIdentity::create(old_root);
    ASSERT_TRUE(identity.has_value());
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        old_root,
        {identity.value(), "app", "./build/app", std::nullopt}));

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("app"), softadastra::ProcessSpec("./build/app", {}, old_root.string()), std::nullopt, identity));
    std::filesystem::rename(old_root, new_root);
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(new_root);
    const char *root_args[] = {"softadastra", "run"};
    softadastra::Cli cli(client);
    EXPECT_EQ(cli.run(2, root_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), "/bin/sh");
    EXPECT_EQ(platform.launcher.last_spec->working_directory(), new_root.string());
    std::filesystem::current_path(new_root / "src");
    ASSERT_TRUE(client.stop_software(softadastra::SoftwareId(identity->value())));
    EXPECT_EQ(cli.run(2, root_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), "/bin/sh");
    EXPECT_EQ(platform.launcher.last_spec->working_directory(), new_root.string());
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(base);
  }

  TEST(CliTest, RunsTomlProjectWithConfiguredIdAndName)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-toml-name-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {softadastra::ProjectIdentity("stable-id"), "phone-test", "sleep 30", std::nullopt}));
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher); softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *arguments[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, arguments), 0);
    std::filesystem::current_path(previous);
    const auto entry = client.software(softadastra::SoftwareId("stable-id"));
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->id().value(), "stable-id");
    EXPECT_EQ(entry->name(), "phone-test");
    EXPECT_EQ(service.find_by_name("phone-test")->id().value(), "stable-id");
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, MigratesLegacyProjectWithInitThenRunsTomlCommand)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-link-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto source = root / "src";
    std::filesystem::create_directories(source);
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    const auto identity = softadastra::ProjectIdentity::create(root);
    ASSERT_TRUE(identity.has_value());
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("legacy"), softadastra::ProcessSpec("--access", {}, "/old/root"), std::nullopt, identity));
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *init_args[] = {"softadastra", "init", "--command", "./build/app", "--access", "http:8080"};
    const char *run_args[] = {"softadastra", "run"};
    softadastra::Cli cli(client);
    EXPECT_EQ(cli.run(6, init_args), 0);
    const auto config = softadastra::ProjectConfigFile::find(root);
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->second.id.value(), "legacy");
    std::filesystem::current_path(source);
    EXPECT_EQ(cli.run(2, run_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->working_directory(), root.string());
    EXPECT_NE(platform.launcher.last_spec->arguments()[1].find("./build/app"), std::string::npos);
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, InitializesAndRunsTomlProjectWithShellCommand)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-init-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *init_args[] = {"softadastra", "init", "demo", "--command", "python3 server.py --port 8000", "--access", "http:8000"};
    ASSERT_EQ(cli.run(7, init_args), 0);
    const auto config = softadastra::ProjectConfigFile::find(root);
    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->second.name, "demo");
    EXPECT_EQ(config->second.command, "python3 server.py --port 8000");
    ASSERT_TRUE(config->second.access.has_value());
    const char *run_args[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, run_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), "/bin/sh");
    EXPECT_NE(platform.launcher.last_spec->arguments()[1].find("python3 server.py --port 8000"), std::string::npos);
    EXPECT_EQ(platform.launcher.last_spec->working_directory(), root.string());
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, TomlConfigurationWinsOverLegacyRegistration)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-toml-wins-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const softadastra::ProjectConfig config{softadastra::ProjectIdentity("toml-stable-id"), "pico", "./build/app", std::nullopt};
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, config));
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("legacy-pico"), softadastra::ProcessSpec("--access")));
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *args[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_NE(platform.launcher.last_spec->arguments()[1].find("./build/app"), std::string::npos);
    EXPECT_TRUE(client.software(softadastra::SoftwareId("toml-stable-id")).has_value());
    std::filesystem::current_path(previous); std::filesystem::remove_all(root);
  }

  TEST(CliTest, ListsAndShowsInfoForRegisteredSoftware)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("api"), softadastra::ProcessSpec("api", {}, "/project/api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8000)));
    const char *run_args[] = {"softadastra", "run", "api"}; EXPECT_EQ(cli.run(3, run_args), 0);
    const char *list_args[] = {"softadastra", "list", "--running"}; EXPECT_EQ(cli.run(3, list_args), 0);
    const char *info_args[] = {"softadastra", "info", "api"}; EXPECT_EQ(cli.run(3, info_args), 0);
  }

  TEST(CliTest, HelpFlagsAreNeverSoftwareNames)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const char *top_help[] = {"softadastra", "--help"};
    const char *access_help[] = {"softadastra", "access", "-h"};
    const char *run_help[] = {"softadastra", "help", "run"};
    EXPECT_EQ(cli.run(2, top_help), 0);
    EXPECT_EQ(cli.run(3, access_help), 0);
    EXPECT_EQ(cli.run(3, run_help), 0);
  }

  TEST(CliTest, ShowsNetworkCapabilityAndNetworkHelp)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const char *info[] = {"softadastra", "network", "info"};
    const char *short_help[] = {"softadastra", "network", "-h"};
    const char *long_help[] = {"softadastra", "network", "--help"};
    const char *help[] = {"softadastra", "help", "network"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, info), 0);
    const auto output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("State:            available"), std::string::npos);
    EXPECT_NE(output.find("Interface:        wlp108s0"), std::string::npos);
    EXPECT_NE(output.find("Managed network:  unavailable"), std::string::npos);
    EXPECT_EQ(cli.run(3, short_help), 0);
    EXPECT_EQ(cli.run(3, long_help), 0);
    EXPECT_EQ(cli.run(3, help), 0);
  }

  TEST(CliTest, ReportsUnavailableManagedNetworkWithoutChangingState)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const char *start[] = {"softadastra", "network", "start"};
    const char *status[] = {"softadastra", "network", "status"};
    const char *stop[] = {"softadastra", "network", "stop"};
    testing::internal::CaptureStderr(); EXPECT_EQ(cli.run(3, start), 1);
    EXPECT_NE(testing::internal::GetCapturedStderr().find("Managed network is unavailable"), std::string::npos);
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, status), 0);
    EXPECT_NE(testing::internal::GetCapturedStdout().find("Managed network: stopped"), std::string::npos);
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, stop), 0);
    EXPECT_NE(testing::internal::GetCapturedStdout().find("is not running"), std::string::npos);
  }

  TEST(CliTest, ResolvesLocalAccessFromCurrentNetworkAndSoftwareState)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("api"), softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.start_software(softadastra::SoftwareId("api")));
    const char *access_api[] = {"softadastra", "access", "api"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 0);
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("State:         running"), std::string::npos);
    EXPECT_NE(output.find("Network:       existing"), std::string::npos);
    EXPECT_NE(output.find("Local URL:     http://10.56.116.55:8080"), std::string::npos);
    EXPECT_NE(output.find("Scan with your phone."), std::string::npos);

    platform.managed_network_.current = {
        softadastra::ManagedNetworkCapability::Available,
        softadastra::ManagedNetworkState::Running, "wlan1", "10.42.0.1",
        "Softadastra-test"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 0);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Network:       existing"), std::string::npos);
    EXPECT_NE(output.find("Local URL:     http://10.56.116.55:8080"), std::string::npos);
    EXPECT_EQ(platform.managed_network_.start_calls, 0);

    platform.network_.capability.primary_ipv4 = "192.168.1.6";
    platform.managed_network_.current.state = softadastra::ManagedNetworkState::Stopped;
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 0);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("http://192.168.1.6:8080"), std::string::npos);

    ASSERT_TRUE(client.stop_software(softadastra::SoftwareId("api")));
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 1);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("State:         stopped"), std::string::npos);
    EXPECT_NE(output.find("Local access:  unavailable"), std::string::npos);
    EXPECT_NE(output.find("softadastra run api"), std::string::npos);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);

    platform.launcher.process_running = false;
    EXPECT_FALSE(client.start_software(softadastra::SoftwareId("api")));
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 1);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("State:         failed"), std::string::npos);
    EXPECT_NE(output.find("softadastra logs api"), std::string::npos);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);
  }

  TEST(CliTest, ExplainsUnavailableNetworkAndSupportsHttpsAccess)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("secure"), softadastra::ProcessSpec("secure"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Https, 8443)));
    ASSERT_TRUE(client.start_software(softadastra::SoftwareId("secure")));
    const char *access_secure[] = {"softadastra", "access", "secure"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_secure), 0);
    auto output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Access:        https:8443"), std::string::npos);
    EXPECT_NE(output.find("https://10.56.116.55:8443"), std::string::npos);

    platform.network_.capability = {softadastra::NetworkState::Unavailable, "", "",
                                    softadastra::NetworkInterfaceType::Unknown,
                                    softadastra::LocalNetworkState::Unavailable,
                                    softadastra::ManagedNetworkCapability::Unavailable};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_secure), 1);
    output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("No local network is available on this Host."), std::string::npos);
    EXPECT_NE(output.find("Managed network: unavailable"), std::string::npos);
    EXPECT_EQ(platform.managed_network_.start_calls, 0);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);
  }

  TEST(CliTest, StartsManagedNetworkOnlyAsFallbackForRunningAccess)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const auto id = softadastra::SoftwareId("api");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.start_software(id));
    platform.network_.capability = {softadastra::NetworkState::Unavailable, "", "", softadastra::NetworkInterfaceType::Unknown, softadastra::LocalNetworkState::Unavailable, softadastra::ManagedNetworkCapability::Available};
    platform.managed_network_.current = {softadastra::ManagedNetworkCapability::Available, softadastra::ManagedNetworkState::Stopped, "wlan1", "10.42.0.1", "Softadastra-test"};
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Started;
    const char *access[] = {"softadastra", "access", "api"};
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access), 0);
    const auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
    EXPECT_NE(output.find("Network:       managed"), std::string::npos);
    EXPECT_NE(output.find("http://10.42.0.1:8080"), std::string::npos);
    EXPECT_NE(output.find("Scan with your phone."), std::string::npos);

    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access), 0);
    static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
  }

  TEST(CliTest, DoesNotStartManagedNetworkForExistingStoppedFailedOrMissingAccess)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const auto api = softadastra::SoftwareId("api"); const auto worker = softadastra::SoftwareId("worker");
    ASSERT_TRUE(client.register_software(api, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.register_software(worker, softadastra::ProcessSpec("worker")));
    platform.managed_network_.current.capability = softadastra::ManagedNetworkCapability::Available;
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Started;
    const char *access_api[] = {"softadastra", "access", "api"}; const char *access_worker[] = {"softadastra", "access", "worker"};
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access_api), 1); static_cast<void>(testing::internal::GetCapturedStdout());
    testing::internal::CaptureStderr(); EXPECT_EQ(cli.run(3, access_worker), 1); static_cast<void>(testing::internal::GetCapturedStderr());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);

    ASSERT_TRUE(client.start_software(api)); ASSERT_TRUE(client.stop_software(api));
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access_api), 1); static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);
    platform.launcher.process_running = false; EXPECT_FALSE(client.start_software(api));
    testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access_api), 1); static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);
  }

  TEST(CliTest, ReportsManagedNetworkStartFailureWithoutUrlOrQr)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    const auto id = softadastra::SoftwareId("api");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080))); ASSERT_TRUE(client.start_software(id));
    platform.network_.capability = {softadastra::NetworkState::Unavailable, "", "", softadastra::NetworkInterfaceType::Unknown, softadastra::LocalNetworkState::Unavailable, softadastra::ManagedNetworkCapability::Available};
    platform.managed_network_.current.capability = softadastra::ManagedNetworkCapability::Available;
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Failed;
    const char *access[] = {"softadastra", "access", "api"}; testing::internal::CaptureStdout(); EXPECT_EQ(cli.run(3, access), 1);
    const auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
    EXPECT_NE(output.find("Unable to start a local Softadastra network."), std::string::npos);
    EXPECT_EQ(output.find("Local URL:"), std::string::npos);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);
  }

  TEST(CliTest, ExplainsMissingAccessPoint)
  {
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("worker"), softadastra::ProcessSpec("worker")));
    const char *access_worker[] = {"softadastra", "access", "worker"};
    testing::internal::CaptureStderr();
    EXPECT_EQ(cli.run(3, access_worker), 1);
    const auto output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("No access configured for: worker"), std::string::npos);
    EXPECT_NE(output.find("global Software has no AccessPoint"), std::string::npos);
  }

  TEST(CliTest, ResolvesProjectAndNamedTargetsUniformly)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-target-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {softadastra::ProjectIdentity("project-app"), "app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)}));
    Platform platform; softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server); softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("project-app"), softadastra::ProcessSpec("--access"), std::nullopt, softadastra::ProjectIdentity("project-app")));
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *start_project[] = {"softadastra", "start"}; const char *stop_project[] = {"softadastra", "stop"};
    const char *restart_project[] = {"softadastra", "restart"}; const char *status_project[] = {"softadastra", "status"};
    const char *info_project[] = {"softadastra", "info"}; const char *access_project[] = {"softadastra", "access"};
    const char *start_named[] = {"softadastra", "start", "project-app"}; const char *stop_named[] = {"softadastra", "stop", "project-app"};
    const char *restart_named[] = {"softadastra", "restart", "project-app"}; const char *status_named[] = {"softadastra", "status", "project-app"};
    const char *info_named[] = {"softadastra", "info", "project-app"}; const char *access_named[] = {"softadastra", "access", "project-app"};
    EXPECT_EQ(cli.run(2, start_project), 0); EXPECT_EQ(cli.run(2, status_project), 0); EXPECT_EQ(cli.run(2, info_project), 0); EXPECT_EQ(cli.run(2, access_project), 0); EXPECT_EQ(cli.run(2, stop_project), 0); EXPECT_EQ(cli.run(2, restart_project), 0);
    EXPECT_EQ(cli.run(3, start_named), 0); EXPECT_EQ(cli.run(3, status_named), 0); EXPECT_EQ(cli.run(3, info_named), 0); EXPECT_EQ(cli.run(3, access_named), 0); EXPECT_EQ(cli.run(3, stop_named), 0); EXPECT_EQ(cli.run(3, restart_named), 0);
    EXPECT_EQ(client.software(softadastra::SoftwareId("project-app"))->process_spec().arguments(), (std::vector<std::string>{"-lc", "sleep 30"}));
    std::filesystem::current_path(previous); std::filesystem::remove_all(root);
  }
}
