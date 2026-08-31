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
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>

namespace
{
  class Process final : public softadastra::Process
  {
  public:
    explicit Process(bool running = true) : running_(running) {}
    bool stop() override
    {
      running_ = false;
      return true;
    }
    [[nodiscard]] bool is_running() const noexcept override { return running_; }
    [[nodiscard]] std::optional<int> exit_code() noexcept override { return std::nullopt; }

  private:
    bool running_{true};
  };

  class Launcher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] softadastra::ProcessLaunchResult launch(const softadastra::ProcessSpec &spec) override
    {
      last_spec = spec;
      return std::make_unique<Process>(process_running);
    }
    std::optional<softadastra::ProcessSpec> last_spec;
    bool process_running{true};
  };

  class Service final : public softadastra::Service
  {
  public:
    bool start() override { return true; }
    bool stop() override { return true; }
    [[nodiscard]] bool is_running() const noexcept override { return true; }
  };

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
        softadastra::ManagedNetworkCapability::Available, "10.56.116.0/24"};
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
        softadastra::ManagedNetworkState::Stopped,
        {},
        {},
        {}};
    softadastra::ManagedNetworkStartResult start_result{
        softadastra::ManagedNetworkStartResult::Unavailable};
    int start_calls{0};
  };

  class Firewall final : public softadastra::LocalFirewall
  {
  public:
    [[nodiscard]] softadastra::LocalFirewallResult ensure(
        const softadastra::LocalFirewallRule &rule) override
    {
      last_rule = rule;
      ++ensure_calls;
      return ensure_result;
    }
    [[nodiscard]] softadastra::LocalFirewallResult status(
        const softadastra::LocalFirewallRule &rule) override
    {
      last_rule = rule;
      ++status_calls;
      return status_result;
    }
    [[nodiscard]] softadastra::LocalFirewallResult allow(
        const softadastra::LocalFirewallRule &rule) override
    {
      last_rule = rule;
      ++allow_calls;
      return allow_result;
    }
    void release(const softadastra::LocalFirewallRule &) noexcept override {}
    [[nodiscard]] softadastra::LocalFirewallResult deny(
        const softadastra::LocalFirewallRule &rule) override
    {
      last_rule = rule;
      ++deny_calls;
      return deny_result;
    }
    softadastra::LocalFirewallResult ensure_result{softadastra::LocalFirewallResult::Open};
    softadastra::LocalFirewallResult status_result{softadastra::LocalFirewallResult::Open};
    softadastra::LocalFirewallResult allow_result{softadastra::LocalFirewallResult::Open};
    softadastra::LocalFirewallResult deny_result{softadastra::LocalFirewallResult::Open};
    softadastra::LocalFirewallRule last_rule;
    int ensure_calls{0};
    int status_calls{0};
    int allow_calls{0};
    int deny_calls{0};
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
    Launcher launcher;
    Service service_;
    Network network_;
    ManagedNetwork managed_network_;
    Firewall firewall_;
    [[nodiscard]] softadastra::LocalFirewall &local_firewall() noexcept override { return firewall_; }
    [[nodiscard]] const softadastra::LocalFirewall &local_firewall() const noexcept override { return firewall_; }
  };

  std::string shell_executable()
  {
#if defined(_WIN32)
    return "cmd.exe";
#else
    return "/bin/sh";
#endif
  }

  std::vector<std::string> shell_arguments(const std::string &command)
  {
#if defined(_WIN32)
    return {"/C", command};
#else
    return {"-lc", command};
#endif
  }

  void expect_same_path(
      const std::optional<std::string> &actual,
      const std::filesystem::path &expected)
  {
    ASSERT_TRUE(actual.has_value());
    const std::filesystem::path actual_path(*actual);
    std::error_code error;

    if (std::filesystem::exists(actual_path, error) &&
        std::filesystem::exists(expected, error))
    {
      EXPECT_TRUE(std::filesystem::equivalent(actual_path, expected, error));
      EXPECT_FALSE(error);
      return;
    }

    EXPECT_EQ(actual_path.lexically_normal(), expected.lexically_normal());
  }

  TEST(CliTest, RunsProjectFromRootAndSubdirectoryUsingCurrentRoot)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-cli-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto root = base / "project";
    std::filesystem::create_directories(root / "src");
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root,
        {"app", "./build/app", std::nullopt, {}}));

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    const softadastra::SoftwareId id("internal-id");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("./build/app", {}, root.string()), std::nullopt, std::nullopt, "app"));
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *root_args[] = {"softadastra", "run"};
    softadastra::Cli cli(client);
    EXPECT_EQ(cli.run(2, root_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), shell_executable());
    expect_same_path(platform.launcher.last_spec->working_directory(), root);
    std::filesystem::current_path(root / "src");
    ASSERT_TRUE(client.stop_software(id));
    EXPECT_EQ(cli.run(2, root_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), shell_executable());
    expect_same_path(platform.launcher.last_spec->working_directory(), root);
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(base);
  }

  TEST(CliTest, RunsTomlProjectWithHostOwnedIdAndName)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-toml-name-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {"phone-test", "sleep 30", std::nullopt, {}}));
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *arguments[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, arguments), 0);
    std::filesystem::current_path(previous);
    const auto entries = client.software();
    ASSERT_EQ(entries.size(), 1U);
    const auto &entry = entries.front();
    EXPECT_EQ(entry.name(), "phone-test");
    EXPECT_FALSE(service.find_by_name("phone-test")->id().value().empty());
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, SynchronizesTomlProjectByHostIdWhenItsNameChanges)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-toml-identity-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const softadastra::SoftwareId id("stable-id");
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root,
        {"first-name", "first-command", std::nullopt, {}}));

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(
        id,
        softadastra::ProcessSpec("old-command", {}, root.string()),
        std::nullopt,
        std::nullopt,
        "first-name"));

    softadastra::Cli cli(client);
    const char *run_args[] = {"softadastra", "run"};
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    ASSERT_EQ(cli.run(2, run_args), 0);
    ASSERT_TRUE(client.stop_software(id));
    ASSERT_TRUE(std::filesystem::remove(root / "softadastra.toml"));
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root,
        {"renamed", "second-command",
         softadastra::AccessPoint::create(
             softadastra::AccessProtocol::Http,
             8080),
         {}}));
    EXPECT_EQ(cli.run(2, run_args), 0);
    std::filesystem::current_path(previous);

    const auto entry = client.software(id);
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->id(), id);
    EXPECT_EQ(entry->name(), "renamed");
    EXPECT_EQ(entry->declared_command(), "second-command");
    ASSERT_TRUE(entry->access_point().has_value());
    EXPECT_EQ(entry->access_point()->port(), 8080);
    EXPECT_EQ(client.software().size(), 1U);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, RejectsDifferentProjectIdUsingRegisteredNameWithoutLaunching)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-name-conflict-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto existing_root = base / "existing";
    const auto current_root = base / "current";
    std::filesystem::create_directories(existing_root);
    std::filesystem::create_directories(current_root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        current_root,
        {"duplicate", "current-command", std::nullopt, {}}));

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(
        softadastra::SoftwareId("existing-id"),
        softadastra::ProcessSpec("existing-command", {}, existing_root.string()),
        std::nullopt,
        softadastra::ProjectIdentity("existing-id"),
        "duplicate"));

    softadastra::Cli cli(client);
    const char *run_args[] = {"softadastra", "run"};
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(current_root);
    testing::internal::CaptureStderr();
    EXPECT_EQ(cli.run(2, run_args), 1);
    const auto output = testing::internal::GetCapturedStderr();
    std::filesystem::current_path(previous);

    EXPECT_NE(output.find("Software name already registered: duplicate"), std::string::npos);
    EXPECT_NE(output.find("Existing project:\n  " + existing_root.string()), std::string::npos);
    EXPECT_NE(output.find("Current project:\n  " + current_root.string()), std::string::npos);
    EXPECT_NE(output.find("Choose another name or remove the existing registration."), std::string::npos);
    ASSERT_TRUE(client.software(softadastra::SoftwareId("existing-id")).has_value());
    EXPECT_FALSE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(client.software().size(), 1U);
    std::filesystem::remove_all(base);
  }

  TEST(CliTest, RegistersDifferentProjectIdWithDifferentName)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-distinct-projects-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto current_root = base / "current";
    std::filesystem::create_directories(current_root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        current_root,
        {"current", "current-command", std::nullopt, {}}));

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(
        softadastra::SoftwareId("existing-id"),
        softadastra::ProcessSpec("existing-command", {}, "/existing"),
        std::nullopt,
        softadastra::ProjectIdentity("existing-id"),
        "existing"));

    softadastra::Cli cli(client);
    const char *run_args[] = {"softadastra", "run"};
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(current_root);
    EXPECT_EQ(cli.run(2, run_args), 0);
    std::filesystem::current_path(previous);

    EXPECT_EQ(client.software().size(), 2U);
    EXPECT_EQ(client.software().size(), 2U);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    expect_same_path(platform.launcher.last_spec->working_directory(), current_root);
    std::filesystem::remove_all(base);
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
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("legacy"), softadastra::ProcessSpec("--access", {}, "/old/root"), std::nullopt, std::nullopt, "legacy"));
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *init_args[] = {"softadastra", "init", "--command", "./build/app", "--access", "http:8080"};
    const char *run_args[] = {"softadastra", "run"};
    softadastra::Cli cli(client);
    EXPECT_EQ(cli.run(6, init_args), 0);
    const auto config = softadastra::ProjectConfigFile::find(root);
    ASSERT_TRUE(config.has_value());
    std::filesystem::current_path(source);
    EXPECT_EQ(cli.run(2, run_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    expect_same_path(platform.launcher.last_spec->working_directory(), root);
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
    std::ifstream input(root / "softadastra.toml");
    const std::string contents{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    EXPECT_EQ(contents.find("id ="), std::string::npos);
    const char *run_args[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, run_args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_EQ(platform.launcher.last_spec->executable(), shell_executable());
    EXPECT_NE(platform.launcher.last_spec->arguments()[1].find("python3 server.py --port 8000"), std::string::npos);
    expect_same_path(platform.launcher.last_spec->working_directory(), root);
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, IgnoresLegacyTomlIdWhenResolvingProjectRoot)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-legacy-id-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const auto toml = root / "softadastra.toml";

    {
      std::ofstream output(toml);
      output << "id = \"old-user-id\"\n"
             << "name = \"legacy\"\n"
             << "command = \"legacy-command\"\n";
    }

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const char *run_args[] = {"softadastra", "run"};
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    ASSERT_EQ(cli.run(2, run_args), 0);
    ASSERT_EQ(client.software().size(), 1U);
    const auto id = client.software().front().id();
    ASSERT_TRUE(client.stop_software(id));

    {
      std::ofstream output(toml);
      output << "id = \"manually-changed-id\"\n"
             << "name = \"legacy\"\n"
             << "command = \"legacy-command\"\n";
    }

    EXPECT_EQ(cli.run(2, run_args), 0);
    std::filesystem::current_path(previous);
    ASSERT_EQ(client.software().size(), 1U);
    EXPECT_EQ(client.software().front().id(), id);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, CopyingLegacyTomlDoesNotCopyHostIdentity)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-legacy-copy-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto first_root = base / "first";
    const auto second_root = base / "second";
    std::filesystem::create_directories(first_root);
    std::filesystem::create_directories(second_root);

    const auto write_config = [](const std::filesystem::path &root,
                                 const std::string &name)
    {
      std::ofstream output(root / "softadastra.toml");
      output << "id = \"copied-legacy-id\"\n"
             << "name = \"" << name << "\"\n"
             << "command = \"copy-command\"\n";
    };

    write_config(first_root, "shop");
    write_config(second_root, "shop");

    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const char *run_args[] = {"softadastra", "run"};
    const auto previous = std::filesystem::current_path();

    std::filesystem::current_path(first_root);
    ASSERT_EQ(cli.run(2, run_args), 0);
    const auto first_id = client.software().front().id();
    ASSERT_TRUE(client.stop_software(first_id));

    std::filesystem::current_path(second_root);
    EXPECT_EQ(cli.run(2, run_args), 1);
    EXPECT_EQ(client.software().size(), 1U);
    EXPECT_FALSE(platform.launcher.last_spec->working_directory() == second_root.string());

    write_config(second_root, "shop-copy");
    EXPECT_EQ(cli.run(2, run_args), 0);
    std::filesystem::current_path(previous);
    ASSERT_EQ(client.software().size(), 2U);
    EXPECT_NE(client.software().back().id(), first_id);
    std::filesystem::remove_all(base);
  }

  TEST(CliTest, TomlConfigurationWinsOverLegacyRegistration)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-toml-wins-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    const softadastra::ProjectConfig config{"pico", "./build/app", std::nullopt, {}};
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, config));
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("legacy-pico"), softadastra::ProcessSpec("--access")));
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *args[] = {"softadastra", "run"};
    EXPECT_EQ(cli.run(2, args), 0);
    ASSERT_TRUE(platform.launcher.last_spec.has_value());
    EXPECT_NE(platform.launcher.last_spec->arguments()[1].find("./build/app"), std::string::npos);
    EXPECT_EQ(client.software().size(), 2U);
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, ListsAndShowsInfoForRegisteredSoftware)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("api"), softadastra::ProcessSpec("api", {}, "/project/api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8000)));
    const char *run_args[] = {"softadastra", "run", "api"};
    EXPECT_EQ(cli.run(3, run_args), 0);
    const char *list_args[] = {"softadastra", "list", "--running"};
    EXPECT_EQ(cli.run(3, list_args), 0);
    const char *info_args[] = {"softadastra", "info", "api"};
    EXPECT_EQ(cli.run(3, info_args), 0);
  }

  TEST(CliTest, HelpFlagsAreNeverSoftwareNames)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const char *top_help[] = {"softadastra", "--help"};
    const char *access_help[] = {"softadastra", "access", "-h"};
    const char *run_help[] = {"softadastra", "help", "run"};
    EXPECT_EQ(cli.run(2, top_help), 0);
    EXPECT_EQ(cli.run(3, access_help), 0);
    EXPECT_EQ(cli.run(3, run_help), 0);
  }

  TEST(CliTest, ShowsNetworkCapabilityAndNetworkHelp)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
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
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const char *start[] = {"softadastra", "network", "start"};
    const char *status[] = {"softadastra", "network", "status"};
    const char *stop[] = {"softadastra", "network", "stop"};
    testing::internal::CaptureStderr();
    EXPECT_EQ(cli.run(3, start), 1);
    EXPECT_NE(testing::internal::GetCapturedStderr().find("Managed network is unavailable"), std::string::npos);
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, status), 0);
    EXPECT_NE(testing::internal::GetCapturedStdout().find("Managed network: stopped"), std::string::npos);
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, stop), 0);
    EXPECT_NE(testing::internal::GetCapturedStdout().find("is not running"), std::string::npos);
  }

  TEST(CliTest, ResolvesLocalAccessFromCurrentNetworkAndSoftwareState)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
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
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
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
                                    softadastra::ManagedNetworkCapability::Unavailable,
                                    {}};
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
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const auto id = softadastra::SoftwareId("api");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.start_software(id));
    platform.network_.capability = {softadastra::NetworkState::Unavailable, "", "",
                                    softadastra::NetworkInterfaceType::Unknown,
                                    softadastra::LocalNetworkState::Unavailable,
                                    softadastra::ManagedNetworkCapability::Available,
                                    {}};
    platform.managed_network_.current = {softadastra::ManagedNetworkCapability::Available, softadastra::ManagedNetworkState::Stopped, "wlan1", "10.42.0.1", "Softadastra-test"};
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Started;
    const char *access[] = {"softadastra", "access", "api"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access), 0);
    const auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
    EXPECT_NE(output.find("Network:       managed"), std::string::npos);
    EXPECT_NE(output.find("http://10.42.0.1:8080"), std::string::npos);
    EXPECT_NE(output.find("Scan with your phone."), std::string::npos);

    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access), 0);
    static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
  }

  TEST(CliTest, DoesNotStartManagedNetworkForExistingStoppedFailedOrMissingAccess)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const auto api = softadastra::SoftwareId("api");
    const auto worker = softadastra::SoftwareId("worker");
    ASSERT_TRUE(client.register_software(api, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.register_software(worker, softadastra::ProcessSpec("worker")));
    platform.managed_network_.current.capability = softadastra::ManagedNetworkCapability::Available;
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Started;
    const char *access_api[] = {"softadastra", "access", "api"};
    const char *access_worker[] = {"softadastra", "access", "worker"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 1);
    static_cast<void>(testing::internal::GetCapturedStdout());
    testing::internal::CaptureStderr();
    EXPECT_EQ(cli.run(3, access_worker), 1);
    static_cast<void>(testing::internal::GetCapturedStderr());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);

    ASSERT_TRUE(client.start_software(api));
    ASSERT_TRUE(client.stop_software(api));
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 1);
    static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);
    platform.launcher.process_running = false;
    EXPECT_FALSE(client.start_software(api));
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access_api), 1);
    static_cast<void>(testing::internal::GetCapturedStdout());
    EXPECT_EQ(platform.managed_network_.start_calls, 0);
  }

  TEST(CliTest, ReportsManagedNetworkStartFailureWithoutUrlOrQr)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    const auto id = softadastra::SoftwareId("api");
    ASSERT_TRUE(client.register_software(id, softadastra::ProcessSpec("api"), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080)));
    ASSERT_TRUE(client.start_software(id));
    platform.network_.capability = {softadastra::NetworkState::Unavailable, "", "",
                                    softadastra::NetworkInterfaceType::Unknown,
                                    softadastra::LocalNetworkState::Unavailable,
                                    softadastra::ManagedNetworkCapability::Available,
                                    {}};
    platform.managed_network_.current.capability = softadastra::ManagedNetworkCapability::Available;
    platform.managed_network_.start_result = softadastra::ManagedNetworkStartResult::Failed;
    const char *access[] = {"softadastra", "access", "api"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, access), 1);
    const auto output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(platform.managed_network_.start_calls, 1);
    EXPECT_NE(output.find("Unable to start a local Softadastra network."), std::string::npos);
    EXPECT_EQ(output.find("Local URL:"), std::string::npos);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);
  }

  TEST(CliTest, ExplainsMissingAccessPoint)
  {
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
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
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8080), {}}));
    Platform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    softadastra::Cli cli(client);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("project-app"), softadastra::ProcessSpec("--access", {}, root.string()), std::nullopt, std::nullopt, "app"));
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *start_project[] = {"softadastra", "run"};
    const char *stop_project[] = {"softadastra", "stop"};
    const char *restart_project[] = {"softadastra", "restart"};
    const char *status_project[] = {"softadastra", "status"};
    const char *info_project[] = {"softadastra", "info"};
    const char *access_project[] = {"softadastra", "access"};
    const char *start_named[] = {"softadastra", "start", "app"};
    const char *stop_named[] = {"softadastra", "stop", "app"};
    const char *restart_named[] = {"softadastra", "restart", "app"};
    const char *status_named[] = {"softadastra", "status", "app"};
    const char *info_named[] = {"softadastra", "info", "app"};
    const char *access_named[] = {"softadastra", "access", "app"};
    EXPECT_EQ(cli.run(2, start_project), 0);
    EXPECT_EQ(cli.run(2, status_project), 0);
    EXPECT_EQ(cli.run(2, info_project), 0);
    EXPECT_EQ(cli.run(2, access_project), 0);
    EXPECT_EQ(cli.run(2, stop_project), 0);
    EXPECT_EQ(cli.run(2, restart_project), 0);
    EXPECT_EQ(cli.run(3, start_named), 0);
    EXPECT_EQ(cli.run(3, status_named), 0);
    EXPECT_EQ(cli.run(3, info_named), 0);
    EXPECT_EQ(cli.run(3, access_named), 0);
    EXPECT_EQ(cli.run(3, stop_named), 0);
    EXPECT_EQ(cli.run(3, restart_named), 0);
    EXPECT_EQ(client.software(softadastra::SoftwareId("project-app"))->process_spec().arguments(), shell_arguments("sleep 30"));
    std::filesystem::current_path(previous);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, ChangesProjectAccessWithoutStartingSoftwareOrHost)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-access-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8083), {}}));
    Platform platform;
    platform.network_.capability.primary_ipv4 = "192.168.1.6";
    platform.network_.capability.local_subnet = "192.168.1.0/24";
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    Firewall firewall;
    softadastra::Cli cli(client, platform.network_, platform.managed_network_, firewall);
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *allow[] = {"softadastra", "access", "allow"};
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, allow), 0);
    const auto output = testing::internal::GetCapturedStdout();
    std::filesystem::current_path(previous);

    EXPECT_NE(output.find("Local access allowed."), std::string::npos);
    EXPECT_NE(output.find("http://192.168.1.6:8083"), std::string::npos);
    EXPECT_EQ(firewall.allow_calls, 1);
    EXPECT_EQ(firewall.last_rule.subnet, "192.168.1.0/24");
    EXPECT_EQ(firewall.last_rule.port, 8083);
    EXPECT_TRUE(firewall.last_rule.owner.starts_with("softadastra:"));
    EXPECT_EQ(client.software().size(), 0U);
    EXPECT_FALSE(platform.launcher.last_spec.has_value());
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, AccessAllowReportsPermissionAndDenyIsExplicit)
  {
    const auto root = std::filesystem::temp_directory_path() / ("softadastra-access-permission-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(
        root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8083), {}}));
    Platform platform;
    platform.network_.capability.local_subnet = "192.168.1.0/24";
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service);
    softadastra::ControlClient client(server);
    Firewall firewall;
    firewall.allow_result = softadastra::LocalFirewallResult::PermissionRequired;
    softadastra::Cli cli(client, platform.network_, platform.managed_network_, firewall);
    const auto previous = std::filesystem::current_path();
    std::filesystem::current_path(root);
    const char *allow[] = {"softadastra", "access", "allow"};
    const char *deny[] = {"softadastra", "access", "deny"};
    testing::internal::CaptureStderr();
    EXPECT_EQ(cli.run(3, allow), 1);
    const auto error = testing::internal::GetCapturedStderr();
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(3, deny), 0);
    const auto output = testing::internal::GetCapturedStdout();
    std::filesystem::current_path(previous);

    EXPECT_EQ(error.find("sudo softadastra access allow"), std::string::npos);
    EXPECT_EQ(firewall.allow_calls, 1);
    EXPECT_EQ(firewall.deny_calls, 1);
    EXPECT_NE(output.find("Local access denied."), std::string::npos);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, AccessReadsFirewallStatusWithoutMutatingOrStarting)
  {
    const auto root = std::filesystem::temp_directory_path() / "softadastra-access-status";
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8084), {}}));
    Platform platform;
    platform.network_.capability.primary_ipv4 = "192.168.1.7";
    platform.network_.capability.local_subnet = "192.168.1.0/24";
    softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server);
    Firewall firewall; softadastra::Cli cli(client, platform.network_, platform.managed_network_, firewall);
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *access[] = {"softadastra", "access"};
    EXPECT_EQ(cli.run(2, access), 0);
    std::filesystem::current_path(previous);
    EXPECT_EQ(firewall.status_calls, 1);
    EXPECT_EQ(firewall.allow_calls, 0);
    EXPECT_EQ(firewall.deny_calls, 0);
    EXPECT_EQ(client.software().size(), 0U);
    EXPECT_FALSE(platform.launcher.last_spec.has_value());
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, NamedAccessUsesRegisteredWorkingDirectory)
  {
    const auto root = std::filesystem::temp_directory_path() / "softadastra-named-access";
    std::filesystem::create_directories(root);
    Platform platform;
    platform.network_.capability.local_subnet = "10.0.0.0/24";
    softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server);
    ASSERT_TRUE(client.register_software(softadastra::SoftwareId("named"), softadastra::ProcessSpec("app", {}, root.string()), softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8090), std::nullopt, "named"));
    Firewall firewall; softadastra::Cli cli(client, platform.network_, platform.managed_network_, firewall);
    const char *access[] = {"softadastra", "access", "named"};
    EXPECT_EQ(cli.run(3, access), 0);
    EXPECT_EQ(firewall.status_calls, 1);
    EXPECT_EQ(firewall.last_rule.owner, [&] { std::uint64_t hash = 1469598103934665603ULL; for (const char character : std::filesystem::weakly_canonical(root).string()) { hash ^= static_cast<unsigned char>(character); hash *= 1099511628211ULL; } std::ostringstream tag; tag << "softadastra:" << std::hex << hash; return tag.str(); }());
    EXPECT_EQ(firewall.allow_calls, 0);
    EXPECT_EQ(firewall.deny_calls, 0);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, RunShowsReachabilityOnlyAfterAllowedFirewallStatus)
  {
    const auto root = std::filesystem::temp_directory_path() / "softadastra-run-firewall";
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8085), {}}));
    Platform platform;
    platform.network_.capability.local_subnet = "192.168.1.0/24";
    softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server);
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *run[] = {"softadastra", "run"};
    softadastra::Cli cli(client, platform.network_, platform.managed_network_, platform.firewall_);
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(2, run), 0);
    const auto output = testing::internal::GetCapturedStdout();
    std::filesystem::current_path(previous);
    EXPECT_NE(output.find("Local URL:"), std::string::npos);
    EXPECT_NE(output.find("Scan with your phone."), std::string::npos);
    EXPECT_EQ(platform.firewall_.status_calls, 1);
    EXPECT_EQ(platform.firewall_.allow_calls, 0);
    EXPECT_EQ(platform.firewall_.deny_calls, 0);
    std::filesystem::remove_all(root);
  }

  TEST(CliTest, RunDoesNotAdvertiseBlockedFirewallAccess)
  {
    const auto root = std::filesystem::temp_directory_path() / "softadastra-run-blocked-firewall";
    std::filesystem::create_directories(root);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(root, {"app", "sleep 30", softadastra::AccessPoint::create(softadastra::AccessProtocol::Http, 8086), {}}));
    Platform platform;
    platform.network_.capability.local_subnet = "192.168.1.0/24";
    platform.firewall_.status_result = softadastra::LocalFirewallResult::PermissionRequired;
    softadastra::Host host(platform); softadastra::HostService service(host, platform.launcher);
    softadastra::ControlServer server(service); softadastra::ControlClient client(server);
    const auto previous = std::filesystem::current_path(); std::filesystem::current_path(root);
    const char *run[] = {"softadastra", "run"};
    softadastra::Cli cli(client, platform.network_, platform.managed_network_, platform.firewall_);
    testing::internal::CaptureStdout();
    EXPECT_EQ(cli.run(2, run), 0);
    const auto output = testing::internal::GetCapturedStdout();
    std::filesystem::current_path(previous);
    EXPECT_NE(output.find("Local access:  unavailable"), std::string::npos);
    EXPECT_NE(output.find("Firewall rule is blocked."), std::string::npos);
    EXPECT_NE(output.find("softadastra access allow"), std::string::npos);
    EXPECT_EQ(output.find("Local URL:"), std::string::npos);
    EXPECT_EQ(output.find("Scan with your phone."), std::string::npos);
    EXPECT_EQ(platform.firewall_.status_calls, 1);
    EXPECT_EQ(platform.firewall_.allow_calls, 0);
    EXPECT_EQ(platform.firewall_.deny_calls, 0);
    ASSERT_EQ(client.software().size(), 1U);
    EXPECT_EQ(client.software().front().state(), softadastra::SoftwareState::Running);
    std::filesystem::remove_all(root);
  }
}
