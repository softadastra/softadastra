/**
 *
 *  @file native_process_launcher_test.cpp
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

#include "control/ControlServer.hpp"
#include "control/LocalControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativeProcess.hpp"
#include "platform/NativeProcessLauncher.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/ProcessLauncher.hpp"
#include "platform/ProcessSpec.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <optional>
#include <string>
#include <thread>

#if defined(__linux__)

#include <cerrno>
#include <csignal>
#include <cstdint>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace
{

  std::optional<int> wait_for_exit_code(
      softadastra::Process &process)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      const auto code = process.exit_code();

      if (code.has_value())
      {
        return code;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(10));
    }

    return std::nullopt;
  }

#if defined(__linux__)

  std::optional<pid_t> read_process_id(
      const std::filesystem::path &path)
  {
    for (int attempt = 0; attempt < 100; ++attempt)
    {
      std::ifstream input(path);
      pid_t process{};

      if (input >> process && process > 0)
      {
        return process;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return std::nullopt;
  }

  bool disappears(pid_t process)
  {
    for (int attempt = 0; attempt < 300; ++attempt)
    {
      errno = 0;

      if (::kill(process, 0) == -1 && errno == ESRCH)
      {
        return true;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return false;
  }

  std::uint16_t unused_loopback_port()
  {
    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);
    EXPECT_GE(descriptor, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = 0;
    EXPECT_EQ(
        ::bind(
            descriptor,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address)),
        0);
    socklen_t length = sizeof(address);
    EXPECT_EQ(
        ::getsockname(
            descriptor,
            reinterpret_cast<sockaddr *>(&address),
            &length),
        0);
    ::close(descriptor);
    return ntohs(address.sin_port);
  }

  bool loopback_port_is_available(std::uint16_t port)
  {
    const int descriptor = ::socket(AF_INET, SOCK_STREAM, 0);

    if (descriptor < 0)
    {
      return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(port);
    const bool available =
        ::bind(
            descriptor,
            reinterpret_cast<const sockaddr *>(&address),
            sizeof(address)) == 0;
    ::close(descriptor);
    return available;
  }

#endif

  TEST(NativeProcessLauncherTest, RejectsEmptyExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    auto process = launcher.launch(
        softadastra::ProcessSpec(""));

    EXPECT_EQ(process, nullptr);
  }

  TEST(NativeProcessLauncherTest, RejectsUnknownExecutable)
  {
    softadastra::NativeProcessLauncher launcher;

    auto process = launcher.launch(
        softadastra::ProcessSpec(
            "softadastra-executable-that-does-not-exist"));

    EXPECT_EQ(process, nullptr);
    EXPECT_EQ(
        process.error(),
        softadastra::ProcessLaunchError::ExecutableNotFound);
  }

  TEST(NativeProcessLauncherTest, RejectsNonExecutableFileWithPermissionDenied)
  {
#if defined(__linux__)
    const auto path = std::filesystem::temp_directory_path() /
                      ("softadastra-non-executable-" +
                       std::to_string(::getpid()));
    {
      std::ofstream output(path);
      output << "#!/bin/sh\nexit 0\n";
    }
    ASSERT_EQ(::chmod(path.c_str(), S_IRUSR | S_IWUSR), 0);

    softadastra::NativeProcessLauncher launcher;
    const auto process = launcher.launch(
        softadastra::ProcessSpec(path.string()));

    EXPECT_EQ(process, nullptr);
    EXPECT_EQ(
        process.error(),
        softadastra::ProcessLaunchError::PermissionDenied);
    std::filesystem::remove(path);
#endif
  }

  TEST(NativeProcessLauncherTest, LaunchesRunningProcess)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());
    EXPECT_FALSE(process->exit_code().has_value());

    EXPECT_TRUE(process->stop());
  }

  TEST(NativeProcessLauncherTest, IsolatesManagedProcessFromTerminalGroup)
  {
#if defined(__linux__)
    softadastra::NativeProcessLauncher launcher;
    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    const auto *native_process = dynamic_cast<softadastra::NativeProcess *>(
        &*process);
    ASSERT_NE(native_process, nullptr);
    const auto native_pid = native_process->native_pid();
    ASSERT_TRUE(native_pid.has_value());
    EXPECT_EQ(
        ::getpgid(native_pid.value()),
        native_pid.value());
    EXPECT_TRUE(process->stop());
#endif
  }

  TEST(NativeProcessLauncherTest, TerminatesManagedProcessWhenParentIsKilled)
  {
#if defined(__linux__)
    const auto output = std::filesystem::temp_directory_path() /
                        ("softadastra-pdeath-" + std::to_string(::getpid()));
    int pipefd[2]{};
    ASSERT_EQ(::pipe(pipefd), 0);

    const pid_t host = ::fork();
    ASSERT_GE(host, 0);

    if (host == 0)
    {
      ::close(pipefd[0]);
      softadastra::NativeProcessLauncher launcher;
      auto process = launcher.launch(
          softadastra::ProcessSpec(
              "sleep",
              {"30"},
              {},
              output.string()));
      const auto *native = dynamic_cast<softadastra::NativeProcess *>(
          process ? &*process : nullptr);
      const pid_t managed =
          native != nullptr && native->native_pid().has_value()
              ? native->native_pid().value()
              : -1;
      static_cast<void>(::write(pipefd[1], &managed, sizeof(managed)));
      ::close(pipefd[1]);
      ::pause();
      ::_exit(1);
    }

    ::close(pipefd[1]);
    pid_t managed = -1;
    ASSERT_EQ(::read(pipefd[0], &managed, sizeof(managed)),
              static_cast<ssize_t>(sizeof(managed)));
    ::close(pipefd[0]);
    ASSERT_GT(managed, 0);

    ASSERT_EQ(::kill(host, SIGKILL), 0);
    int status = 0;
    ASSERT_EQ(::waitpid(host, &status, 0), host);

    bool exited = false;

    for (int attempt = 0; attempt < 100; ++attempt)
    {
      errno = 0;

      if (::kill(managed, 0) == -1 && errno == ESRCH)
      {
        exited = true;
        break;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!exited)
    {
      static_cast<void>(::kill(-managed, SIGKILL));
    }

    EXPECT_TRUE(exited);
    std::filesystem::remove(output);
#endif
  }

  TEST(NativeProcessLauncherTest, StopsEveryProcessInTheManagedGroup)
  {
#if defined(__linux__)
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-process-group-" +
                            std::to_string(::getpid()));
    const auto child_file = directory / "child";
    const auto output = directory / "output";
    ASSERT_TRUE(std::filesystem::create_directories(directory));

    softadastra::NativeProcessLauncher launcher;
    const auto command =
        "sleep 30 & echo $! > " + child_file.string() + "; wait";
    auto process = launcher.launch(
        softadastra::ProcessSpec(
            "sh",
            {"-c", command},
            {},
            output.string()));
    ASSERT_NE(process, nullptr);
    const auto child = read_process_id(child_file);
    ASSERT_TRUE(child.has_value());

    EXPECT_TRUE(process->stop());
    EXPECT_TRUE(disappears(*child));
    std::filesystem::remove_all(directory);
#endif
  }

  TEST(NativeProcessLauncherTest, TerminatesShellDescendantWhenHostIsKilled)
  {
#if defined(__linux__)
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-supervisor-" +
                            std::to_string(::getpid()));
    const auto child_file = directory / "child";
    const auto output = directory / "output";
    ASSERT_TRUE(std::filesystem::create_directories(directory));
    const auto port = unused_loopback_port();
    ASSERT_GT(port, 0);

    int pipefd[2]{};
    ASSERT_EQ(::pipe(pipefd), 0);
    const pid_t host = ::fork();
    ASSERT_GE(host, 0);

    if (host == 0)
    {
      ::close(pipefd[0]);
      softadastra::NativeProcessLauncher launcher;
      const auto command =
          std::string(SOFTADASTRA_TEST_APP) +
          " --stay --listen " + std::to_string(port) +
          " & echo $! > " + child_file.string() + "; wait";
      auto process = launcher.launch(
          softadastra::ProcessSpec(
              "sh",
              {"-c", command},
              {},
              output.string()));
      const auto *native = dynamic_cast<softadastra::NativeProcess *>(
          process ? &*process : nullptr);
      const pid_t supervisor =
          native != nullptr && native->native_pid().has_value()
              ? native->native_pid().value()
              : -1;
      static_cast<void>(::write(pipefd[1], &supervisor, sizeof(supervisor)));
      ::close(pipefd[1]);
      ::pause();
      ::_exit(1);
    }

    ::close(pipefd[1]);
    pid_t supervisor = -1;
    ASSERT_EQ(::read(pipefd[0], &supervisor, sizeof(supervisor)),
              static_cast<ssize_t>(sizeof(supervisor)));
    ::close(pipefd[0]);
    ASSERT_GT(supervisor, 0);
    const auto child = read_process_id(child_file);
    ASSERT_TRUE(child.has_value());
    const pid_t process_group = ::getpgid(*child);
    ASSERT_GT(process_group, 0);

    for (int attempt = 0;
         attempt < 100 && loopback_port_is_available(port);
         ++attempt)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ASSERT_FALSE(loopback_port_is_available(port));
    ASSERT_EQ(::kill(host, SIGKILL), 0);
    ASSERT_EQ(::waitpid(host, nullptr, 0), host);

    const bool supervisor_gone = disappears(supervisor);
    const bool child_gone = disappears(*child);

    for (int attempt = 0;
         attempt < 300 && !loopback_port_is_available(port);
         ++attempt)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!supervisor_gone || !child_gone)
    {
      static_cast<void>(::kill(-process_group, SIGKILL));
    }

    EXPECT_TRUE(supervisor_gone);
    EXPECT_TRUE(child_gone);
    EXPECT_TRUE(loopback_port_is_available(port));
    std::filesystem::remove_all(directory);
#endif
  }

  TEST(NativeProcessLauncherTest, ClosesHostDescriptorsBeforeExec)
  {
#if defined(__linux__)
    const auto directory = std::filesystem::temp_directory_path() /
                           ("softadastra-fd-inheritance-" +
                            std::to_string(::getpid()));
    const auto socket_path = directory / "control.sock";
    const auto output = directory / "output";
    ASSERT_TRUE(std::filesystem::create_directories(directory));

    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer server(service);
    softadastra::LocalControlServer control(server, socket_path);
    ASSERT_TRUE(control.start());

    int listener = -1;

    for (int descriptor = 3;
         descriptor < 128;
         ++descriptor)
    {
      sockaddr_un address{};
      socklen_t length = sizeof(address);

      if (::getsockname(
              descriptor,
              reinterpret_cast<sockaddr *>(&address),
              &length) == 0 &&
          address.sun_family == AF_UNIX &&
          socket_path.string() == address.sun_path)
      {
        listener = descriptor;
        break;
      }
    }

    ASSERT_GE(listener, 0);
    EXPECT_NE(::fcntl(listener, F_GETFD) & FD_CLOEXEC, 0);

    const int arbitrary = ::open("/dev/null", O_RDONLY);
    ASSERT_GE(arbitrary, 3);

    softadastra::NativeProcessLauncher launcher;
    const auto command =
        "test -e /proc/self/fd/0 && "
        "test -e /proc/self/fd/1 && "
        "test -e /proc/self/fd/2 && "
        "test ! -e /proc/self/fd/" + std::to_string(listener) +
        " && test ! -e /proc/self/fd/" + std::to_string(arbitrary);
    auto process = launcher.launch(
        softadastra::ProcessSpec(
            "sh",
            {"-c", command},
            {},
            output.string()));
    ASSERT_NE(process, nullptr);
    EXPECT_EQ(wait_for_exit_code(*process), 0);

    ::close(arbitrary);
    control.stop();
    std::filesystem::remove_all(directory);
#endif
  }

  TEST(NativeProcessLauncherTest, ReportsSuccessfulNaturalExit)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "exit 0",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "exit 0",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code = wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 0);
    EXPECT_FALSE(process->is_running());
  }

  TEST(NativeProcessLauncherTest, ReportsFailedNaturalExit)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "exit 7",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "exit 7",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code = wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 7);
    EXPECT_FALSE(process->is_running());
  }

#if defined(_WIN32)

  TEST(
      NativeProcessLauncherTest,
      LaunchesQuotedCmdCommandWithSpacedExecutableAndArguments)
  {
    const auto directory =
        std::filesystem::temp_directory_path() /
        ("softadastra launcher " +
         std::to_string(
             std::chrono::steady_clock::now()
                 .time_since_epoch()
                 .count()));
    const auto executable =
        directory / "fixture with spaces.exe";
    const auto output = directory / "output.txt";

    ASSERT_TRUE(std::filesystem::create_directories(directory));
    ASSERT_TRUE(std::filesystem::copy_file(
        SOFTADASTRA_TEST_APP,
        executable));

    const std::string command =
        "\"" + executable.string() +
        "\" --stdout stdout-value --stderr \"stderr value\""
        " --echo plain \"argument with spaces\"";
    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {"/C", command},
        directory.string(),
        output.string());
    softadastra::NativeProcessLauncher launcher;

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);

    const auto code = wait_for_exit_code(*process);

    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code.value(), 0);

    std::ifstream input(output);
    const std::string logs{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()};
    input.close();

    EXPECT_NE(logs.find("stdout-value"), std::string::npos);
    EXPECT_NE(logs.find("stderr value"), std::string::npos);
    EXPECT_NE(logs.find("plain"), std::string::npos);
    EXPECT_NE(logs.find("argument with spaces"), std::string::npos);

    std::filesystem::remove_all(directory);
  }

#endif

  TEST(NativeProcessLauncherTest, PreservesArguments)
  {
    softadastra::NativeProcessLauncher launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());

    EXPECT_TRUE(process->stop());
  }

  TEST(NativeProcessLauncherTest, SupportsProcessLauncherInterface)
  {
    softadastra::NativeProcessLauncher native_launcher;
    softadastra::ProcessLauncher &launcher = native_launcher;

#if defined(_WIN32)

    const softadastra::ProcessSpec spec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    const softadastra::ProcessSpec spec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif

    auto process = launcher.launch(spec);

    ASSERT_NE(process, nullptr);
    EXPECT_TRUE(process->is_running());

    EXPECT_TRUE(process->stop());
  }

} // namespace
