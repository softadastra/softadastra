/**
 *
 *  @file local_control_test.cpp
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

#include <gtest/gtest.h>
#include "control/ControlClient.hpp"
#include "host/HostStateFile.hpp"
#include "software/ProjectConfig.hpp"

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#if defined(__linux__)

#include <csignal>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
  struct CommandResult
  {
    int exit_code;
    std::string output;
  };

  bool wait_for(const std::function<bool()> &predicate)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      if (predicate())
      {
        return true;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return predicate();
  }

  CommandResult invoke_cli(
      const std::filesystem::path &state_home,
      const std::vector<std::string> &arguments,
      const std::filesystem::path &working_directory = {})
  {
    int output_pipe[2]{};
    EXPECT_EQ(::pipe(output_pipe), 0);
    const pid_t child = ::fork();

    if (child == 0)
    {
      ::setenv("XDG_STATE_HOME", state_home.c_str(), 1);
      if (!working_directory.empty())
        static_cast<void>(::chdir(working_directory.c_str()));
      ::dup2(output_pipe[1], STDOUT_FILENO);
      ::dup2(output_pipe[1], STDERR_FILENO);
      ::close(output_pipe[0]);
      ::close(output_pipe[1]);
      std::vector<char *> values;
      values.reserve(arguments.size() + 2);
      values.push_back(const_cast<char *>(SOFTADASTRA_EXECUTABLE));

      for (const auto &argument : arguments)
      {
        values.push_back(const_cast<char *>(argument.c_str()));
      }

      values.push_back(nullptr);
      ::execv(SOFTADASTRA_EXECUTABLE, values.data());
      ::_exit(127);
    }

    ::close(output_pipe[1]);
    std::string output;
    char buffer[256];
    ssize_t received = 0;

    while ((received = ::read(output_pipe[0], buffer, sizeof(buffer))) > 0)
    {
      output.append(buffer, static_cast<std::size_t>(received));
    }

    ::close(output_pipe[0]);
    int status = 0;
    EXPECT_EQ(::waitpid(child, &status, 0), child);
    return CommandResult{WIFEXITED(status) ? WEXITSTATUS(status) : 1, output};
  }

  class HostProcess
  {
  public:
    explicit HostProcess(const std::filesystem::path &state_home)
    {
      int output_pipe[2]{};
      EXPECT_EQ(::pipe(output_pipe), 0);
      process_ = ::fork();

      if (process_ == 0)
      {
        ::setenv("XDG_STATE_HOME", state_home.c_str(), 1);
        ::dup2(output_pipe[1], STDOUT_FILENO);
        ::dup2(output_pipe[1], STDERR_FILENO);
        ::close(output_pipe[0]);
        ::close(output_pipe[1]);
        ::execl(SOFTADASTRA_EXECUTABLE, SOFTADASTRA_EXECUTABLE, "host", nullptr);
        ::_exit(127);
      }

      ::close(output_pipe[1]);
      output_descriptor_ = output_pipe[0];
      const int flags = ::fcntl(output_descriptor_, F_GETFL);

      if (flags != -1)
      {
        static_cast<void>(
            ::fcntl(output_descriptor_, F_SETFL, flags | O_NONBLOCK));
      }
    }

    ~HostProcess()
    {
      stop();

      if (output_descriptor_ >= 0)
      {
        ::close(output_descriptor_);
      }
    }

    [[nodiscard]] bool valid() const noexcept
    {
      return process_ > 0;
    }

    void stop()
    {
      if (process_ <= 0)
      {
        return;
      }

      static_cast<void>(::kill(process_, SIGTERM));
      int status = 0;
      static_cast<void>(::waitpid(process_, &status, 0));
      process_ = -1;
    }

    [[nodiscard]] std::string output() const
    {
      pollfd descriptor{output_descriptor_, POLLIN, 0};
      static_cast<void>(::poll(&descriptor, 1, 1000));
      std::string result;
      char buffer[256];
      ssize_t received = 0;

      while ((received = ::read(output_descriptor_, buffer, sizeof(buffer))) > 0)
      {
        result.append(buffer, static_cast<std::size_t>(received));
      }

      return result;
    }

  private:
    pid_t process_{-1};
    int output_descriptor_{-1};
  };

  TEST(LocalControlTest, PreservesRegisteredNameAcrossHostRestart)
  {
    const auto state_home = std::filesystem::temp_directory_path() /
                            ("softadastra-local-control-" + std::to_string(
                                std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()));
    const auto socket = state_home / "softadastra" / "control.sock";
    const auto project = state_home / "project";
    const auto source = project / "src";
    std::filesystem::remove_all(state_home);
    std::filesystem::create_directories(project);
    std::filesystem::create_directories(source);

    HostProcess host(state_home);
    ASSERT_TRUE(host.valid());
    ASSERT_TRUE(wait_for([&socket]()
                         {
                           return std::filesystem::exists(socket);
                         }));
    const auto started = host.output();
    EXPECT_NE(started.find("Softadastra Host is running"), std::string::npos);
    EXPECT_NE(started.find("hostname: "), std::string::npos);
    EXPECT_NE(started.find("local control: ready"), std::string::npos);
    EXPECT_NE(started.find("network: "), std::string::npos);
    EXPECT_NE(started.find("ipv4: "), std::string::npos);
    EXPECT_NE(started.find("local name: "), std::string::npos);
    EXPECT_NE(started.find("remote access: disabled"), std::string::npos);
    EXPECT_NE(started.find("Press Ctrl+C to stop."), std::string::npos);

    EXPECT_EQ(invoke_cli(state_home, {"register", "demo", "--access", "http:8080", "--", "sleep", "30"}, project).exit_code, 0);
    const auto run_from_root = invoke_cli(state_home, {"run", "demo"}, project);
    EXPECT_NE(run_from_root.output.find("running: demo"), std::string::npos);
    softadastra::ControlClient local_client(socket);
    const auto target = local_client.local_gateway_target("demo.softadastra.home.arpa");
    EXPECT_EQ(target.result, softadastra::LocalGatewayLookup::Http);
    EXPECT_EQ(target.port, 8080);
    const auto run_from_subdirectory = invoke_cli(state_home, {"run", "demo"}, source);
    EXPECT_EQ(run_from_subdirectory.exit_code, 0);
    EXPECT_NE(run_from_subdirectory.output.find("already running: demo"), std::string::npos);
    const auto running = invoke_cli(state_home, {"status", "demo"});
    EXPECT_EQ(running.exit_code, 0);
    EXPECT_NE(running.output.find("demo: running"), std::string::npos);
    EXPECT_EQ(invoke_cli(state_home, {"stop", "demo"}).exit_code, 0);
    const auto stopped = invoke_cli(state_home, {"status", "demo"});
    EXPECT_EQ(stopped.exit_code, 0);
    EXPECT_NE(stopped.output.find("demo: stopped"), std::string::npos);
    EXPECT_EQ(invoke_cli(state_home, {"restart", "demo"}).exit_code, 0);
    const auto restarted = invoke_cli(state_home, {"status", "demo"});
    EXPECT_EQ(restarted.exit_code, 0);
    EXPECT_NE(restarted.output.find("demo: running"), std::string::npos);
    EXPECT_EQ(invoke_cli(state_home, {"stop", "demo"}).exit_code, 0);

    host.stop();
    const auto shutdown_output = host.output();
    EXPECT_NE(
        shutdown_output.find("Stopping Softadastra Host..."),
        std::string::npos);
    EXPECT_NE(
        shutdown_output.find("Softadastra Host stopped."),
        std::string::npos);
    EXPECT_FALSE(std::filesystem::exists(socket));

    HostProcess restarted_host(state_home);
    ASSERT_TRUE(restarted_host.valid());
    ASSERT_TRUE(wait_for([&socket]() { return std::filesystem::exists(socket); }));
    EXPECT_EQ(invoke_cli(state_home, {"status", "demo"}).exit_code, 0);
    restarted_host.stop();

    softadastra::HostState restored;
    softadastra::HostStateFile state_file(state_home / "softadastra" / "host-state");
    ASSERT_TRUE(state_file.load(restored));
    const auto *registered = restored.find_software_by_name("demo");
    ASSERT_NE(registered, nullptr);
    EXPECT_EQ(registered->name(), "demo");
    EXPECT_EQ(registered->id(), softadastra::SoftwareId("demo"));
    std::filesystem::remove_all(state_home);
  }

  TEST(LocalControlTest, PreservesTomlNameAcrossHostRestart)
  {
    const auto state_home = std::filesystem::temp_directory_path() / ("softadastra-toml-restart-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto project = state_home / "project"; const auto socket = state_home / "softadastra" / "control.sock";
    std::filesystem::create_directories(project);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(project, {softadastra::ProjectIdentity("stable-id"), "phone-test", "sleep 30", std::nullopt}));
    HostProcess host(state_home); ASSERT_TRUE(wait_for([&socket] { return std::filesystem::exists(socket); }));
    ASSERT_EQ(invoke_cli(state_home, {"run"}, project).exit_code, 0);
    softadastra::ControlClient before_restart(socket);
    const auto entries_after_run = before_restart.software();
    ASSERT_EQ(entries_after_run.size(), 1U);
    const auto after_run = before_restart.software(softadastra::SoftwareId("stable-id"));
    ASSERT_TRUE(after_run.has_value());
    EXPECT_EQ(after_run->id().value(), "stable-id");
    EXPECT_EQ(after_run->name(), "phone-test");
    EXPECT_EQ(after_run->process_spec().working_directory(), project.string());
    EXPECT_EQ(after_run->declared_command(), "sleep 30");
    EXPECT_FALSE(after_run->access_point().has_value());
    host.stop();
    softadastra::HostState after_first_stop;
    const softadastra::HostStateFile state_file(state_home / "softadastra" / "host-state");
    ASSERT_TRUE(state_file.load(after_first_stop));
    ASSERT_EQ(after_first_stop.software_count(), 1U);
    const auto *persisted = after_first_stop.find_software_by_name("phone-test");
    ASSERT_NE(persisted, nullptr);
    EXPECT_EQ(persisted->id().value(), "stable-id");
    EXPECT_EQ(persisted->process_spec().working_directory(), project.string());
    EXPECT_EQ(persisted->declared_command(), "sleep 30");
    EXPECT_FALSE(persisted->access_point().has_value());
    HostProcess restarted(state_home); ASSERT_TRUE(wait_for([&socket] { return std::filesystem::exists(socket); }));
    softadastra::ControlClient after_restart_client(socket);
    const auto entries_after_restart = after_restart_client.software();
    ASSERT_EQ(entries_after_restart.size(), 1U);
    const auto after_restart = after_restart_client.software(softadastra::SoftwareId("stable-id"));
    ASSERT_TRUE(after_restart.has_value());
    EXPECT_EQ(after_restart->id().value(), "stable-id");
    EXPECT_EQ(after_restart->name(), "phone-test");
    EXPECT_EQ(after_restart->process_spec().working_directory(), project.string());
    EXPECT_EQ(after_restart->declared_command(), "sleep 30");
    EXPECT_FALSE(after_restart->access_point().has_value());
    ASSERT_EQ(invoke_cli(state_home, {"status", "stable-id"}).exit_code, 0);
    restarted.stop();
    softadastra::HostState restored; ASSERT_TRUE(state_file.load(restored));
    const auto *entry=restored.find_software_by_name("phone-test"); ASSERT_NE(entry,nullptr);
    EXPECT_EQ(entry->id().value(), "stable-id"); EXPECT_EQ(entry->name(), "phone-test");
    std::filesystem::remove_all(state_home);
  }

  TEST(LocalControlTest, PreservesRenamedTomlNameAcrossHostRestart)
  {
    const auto state_home = std::filesystem::temp_directory_path() / ("softadastra-rename-restart-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto project = state_home / "project"; const auto socket = state_home / "softadastra" / "control.sock";
    std::filesystem::create_directories(project);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(project, {softadastra::ProjectIdentity("stable-id"), "phone-test", "sleep 30", std::nullopt}));
    HostProcess host(state_home); ASSERT_TRUE(wait_for([&socket] { return std::filesystem::exists(socket); }));
    ASSERT_EQ(invoke_cli(state_home, {"run"}, project).exit_code, 0);
    const auto toml = project / "softadastra.toml";
    std::filesystem::remove(toml);
    ASSERT_TRUE(softadastra::ProjectConfigFile::create(project, {softadastra::ProjectIdentity("stable-id"), "phone-api", "sleep 30", std::nullopt}));
    ASSERT_EQ(invoke_cli(state_home, {"run"}, project).exit_code, 0);
    host.stop();
    HostProcess restarted(state_home); ASSERT_TRUE(wait_for([&socket] { return std::filesystem::exists(socket); }));
    restarted.stop();
    softadastra::HostState restored; ASSERT_TRUE(softadastra::HostStateFile(state_home / "softadastra" / "host-state").load(restored));
    EXPECT_EQ(restored.find_software_by_name("phone-test"), nullptr);
    const auto *entry=restored.find_software_by_name("phone-api"); ASSERT_NE(entry,nullptr);
    EXPECT_EQ(entry->id().value(), "stable-id"); EXPECT_EQ(entry->name(), "phone-api");
    std::filesystem::remove_all(state_home);
  }
} // namespace

#else

TEST(LocalControlTest, RequiresLinuxLocalChannel)
{
  GTEST_SKIP();
}

#endif
