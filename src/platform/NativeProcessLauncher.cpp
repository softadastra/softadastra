/**
 *
 *  @file NativeProcessLauncher.cpp
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

#include "platform/NativeProcessLauncher.hpp"

#include "platform/NativeProcess.hpp"

#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>
#include <vix/error/ErrorCode.hpp>
#include <vix/process/Command.hpp>
#include <vix/process/Spawn.hpp>

#if defined(__linux__)

#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace softadastra
{
  namespace
  {
    bool executable_exists(const std::string &executable)
    {
      const std::filesystem::path path(executable);

      if (path.has_parent_path())
      {
        return std::filesystem::exists(path);
      }

      const char *environment_path = std::getenv("PATH");

      if (environment_path == nullptr)
      {
        return false;
      }

#if defined(_WIN32)
      constexpr char separator = ';';
#else
      constexpr char separator = ':';
#endif

      std::string_view directories(environment_path);

      while (!directories.empty())
      {
        const auto position = directories.find(separator);
        const auto directory = directories.substr(0, position);

        if (std::filesystem::exists(
                std::filesystem::path(directory) / path))
        {
          return true;
        }

        if (position == std::string_view::npos)
        {
          break;
        }

        directories.remove_prefix(position + 1);
      }

      return false;
    }

  } // namespace

  ProcessLaunchResult NativeProcessLauncher::launch(
      const ProcessSpec &spec)
  {
    if (spec.executable().empty())
    {
      return ProcessLaunchError::LaunchFailed;
    }

    if (!executable_exists(spec.executable()))
    {
      return ProcessLaunchError::ExecutableNotFound;
    }

#if defined(__linux__)
    if (spec.output_file().has_value())
    {
      const pid_t pid = ::fork();
      if (pid < 0) return ProcessLaunchError::LaunchFailed;
      if (pid == 0)
      {
        ::setsid();
        const int log = ::open(spec.output_file()->c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log < 0) _exit(126);
        ::dup2(log, STDOUT_FILENO); ::dup2(log, STDERR_FILENO); if (log > STDERR_FILENO) ::close(log);
        if (spec.working_directory().has_value() && ::chdir(spec.working_directory()->c_str()) != 0) { ::dprintf(STDERR_FILENO, "softadastra: cannot use working directory: %s\n", std::strerror(errno)); _exit(126); }
        std::vector<char *> args; args.push_back(const_cast<char *>(spec.executable().c_str())); for (const auto &argument : spec.arguments()) args.push_back(const_cast<char *>(argument.c_str())); args.push_back(nullptr);
        ::execvp(args[0], args.data()); ::dprintf(STDERR_FILENO, "softadastra: cannot start command: %s\n", std::strerror(errno)); _exit(127);
      }
      return std::make_unique<NativeProcess>(pid);
    }
#endif

    vix::process::Command command(spec.executable());

    command.args(spec.arguments());
    if (spec.working_directory().has_value())
      command.cwd(spec.working_directory().value());
    command.search_in_path(true);
    command.detach(true);

#if defined(__linux__)

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    sigset_t previous_signals;
    const bool reset_signals = pthread_sigmask(
                                   SIG_UNBLOCK,
                                   &signals,
                                   &previous_signals) == 0;

#endif

    auto result = vix::process::spawn(std::move(command));

#if defined(__linux__)

    if (reset_signals)
    {
      pthread_sigmask(SIG_SETMASK, &previous_signals, nullptr);
    }

#endif

    if (!result)
    {
      switch (result.error().code())
      {
      case vix::error::ErrorCode::NotFound:
        return ProcessLaunchError::ExecutableNotFound;

      case vix::error::ErrorCode::PermissionDenied:
        return ProcessLaunchError::PermissionDenied;

      default:
        return ProcessLaunchError::LaunchFailed;
      }
    }

    vix::process::Child child = std::move(result.value());

    if (!child.valid())
    {
      return ProcessLaunchError::LaunchFailed;
    }

    return std::make_unique<NativeProcess>(
        std::move(child));

  }

} // namespace softadastra
