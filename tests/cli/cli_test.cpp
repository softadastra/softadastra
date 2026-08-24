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
    bool stop() override { running_ = false; return true; }
    [[nodiscard]] bool is_running() const noexcept override { return running_; }
    [[nodiscard]] std::optional<int> exit_code() noexcept override { return std::nullopt; }
  private: bool running_{true};
  };

  class Launcher final : public softadastra::ProcessLauncher
  {
  public:
    [[nodiscard]] softadastra::ProcessLaunchResult launch(const softadastra::ProcessSpec &spec) override
    { last_spec = spec; return std::make_unique<Process>(); }
    std::optional<softadastra::ProcessSpec> last_spec;
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
    Launcher launcher; Service service_; Network network_;
  };

  TEST(CliTest, RunsMovedProjectFromRootAndSubdirectoryUsingCurrentRoot)
  {
    const auto base = std::filesystem::temp_directory_path() / ("softadastra-cli-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto old_root = base / "old";
    const auto new_root = base / "new";
    std::filesystem::create_directories(old_root / "src");
    const auto identity = softadastra::ProjectIdentity::create(old_root);
    ASSERT_TRUE(identity.has_value());

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
    EXPECT_EQ(platform.launcher.last_spec->executable(), "./build/app");
    EXPECT_EQ(platform.launcher.last_spec->working_directory(), new_root.string());
    std::filesystem::current_path(new_root / "src");
    EXPECT_EQ(cli.run(2, root_args), 0);
    std::filesystem::current_path(previous);
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
    EXPECT_EQ(platform.launcher.last_spec->arguments(), (std::vector<std::string>{"-lc", "./build/app"}));
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
    EXPECT_EQ(platform.launcher.last_spec->arguments(), (std::vector<std::string>{"-lc", "python3 server.py --port 8000"}));
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
    EXPECT_EQ(platform.launcher.last_spec->arguments(), (std::vector<std::string>{"-lc", "./build/app"}));
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
