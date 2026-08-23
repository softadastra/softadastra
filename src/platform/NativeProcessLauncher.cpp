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

    vix::process::Command command(spec.executable());

    command.args(spec.arguments());
    command.search_in_path(true);
    command.detach(false);

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
