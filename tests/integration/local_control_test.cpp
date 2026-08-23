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

#include <chrono>
#include <filesystem>
#include <functional>
#include <string>
#include <thread>
#include <vector>

#if defined(__linux__)

#include <csignal>
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
      const std::vector<std::string> &arguments)
  {
    int output_pipe[2]{};
    EXPECT_EQ(::pipe(output_pipe), 0);
    const pid_t child = ::fork();

    if (child == 0)
    {
      ::setenv("XDG_STATE_HOME", state_home.c_str(), 1);
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
      process_ = ::fork();

      if (process_ == 0)
      {
        ::setenv("XDG_STATE_HOME", state_home.c_str(), 1);
        ::execl(SOFTADASTRA_EXECUTABLE, SOFTADASTRA_EXECUTABLE, "host", nullptr);
        ::_exit(127);
      }
    }

    ~HostProcess()
    {
      stop();
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

  private:
    pid_t process_{-1};
  };

  TEST(LocalControlTest, ControlsPersistentHostFromSeparateCliProcesses)
  {
    const auto state_home = std::filesystem::temp_directory_path() /
                            ("softadastra-local-control-" + std::to_string(
                                std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()));
    const auto socket = state_home / "softadastra" / "control.sock";
    std::filesystem::remove_all(state_home);

    const auto absent = invoke_cli(state_home, {"status", "demo"});
    EXPECT_EQ(absent.exit_code, 1);
    EXPECT_NE(absent.output.find("Host is not running"), std::string::npos);

    HostProcess host(state_home);
    ASSERT_TRUE(host.valid());
    ASSERT_TRUE(wait_for([&socket]()
                         {
                           return std::filesystem::exists(socket);
                         }));

    const auto access = invoke_cli(state_home, {"access"});
    EXPECT_EQ(access.exit_code, 0);
    EXPECT_NE(access.output.find("hosted software endpoints"), std::string::npos);

    EXPECT_EQ(invoke_cli(state_home, {"register", "demo", "sleep", "30"}).exit_code, 0);
    EXPECT_EQ(invoke_cli(state_home, {"start", "demo"}).exit_code, 0);
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
    EXPECT_FALSE(std::filesystem::exists(socket));
    std::filesystem::remove_all(state_home);
  }
} // namespace

#else

TEST(LocalControlTest, RequiresLinuxLocalChannel)
{
  GTEST_SKIP();
}

#endif
