/**
 *
 *  @file NativeProcess.cpp
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

#include "platform/NativeProcess.hpp"

#if defined(_WIN32)

#include <windows.h>

#else

#include <cerrno>
#include <csignal>
#include <limits>
#include <sys/types.h>

#endif

namespace softadastra
{

  NativeProcess::NativeProcess(Id id) noexcept
      : id_(id)
  {
  }

  bool NativeProcess::stop()
  {
    if (!is_running())
    {
      return true;
    }

#if defined(_WIN32)

    if (id_ > static_cast<Id>(MAXDWORD))
    {
      return false;
    }

    HANDLE process = OpenProcess(
        PROCESS_TERMINATE | SYNCHRONIZE,
        FALSE,
        static_cast<DWORD>(id_));

    if (process == nullptr)
    {
      return false;
    }

    const BOOL terminated = TerminateProcess(process, 1);

    if (terminated != FALSE)
    {
      WaitForSingleObject(process, 5000);
    }

    CloseHandle(process);

    return terminated != FALSE;

#else

    if (id_ == 0 ||
        id_ > static_cast<Id>(std::numeric_limits<pid_t>::max()))
    {
      return false;
    }

    const pid_t pid = static_cast<pid_t>(id_);

    if (::kill(pid, SIGTERM) == 0)
    {
      return true;
    }

    return errno == ESRCH;

#endif
  }

  bool NativeProcess::is_running() const noexcept
  {
    if (id_ == 0)
    {
      return false;
    }

#if defined(_WIN32)

    if (id_ > static_cast<Id>(MAXDWORD))
    {
      return false;
    }

    HANDLE process = OpenProcess(
        PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE,
        FALSE,
        static_cast<DWORD>(id_));

    if (process == nullptr)
    {
      return false;
    }

    DWORD exit_code = 0;

    const BOOL queried =
        GetExitCodeProcess(process, &exit_code);

    CloseHandle(process);

    if (queried == FALSE)
    {
      return false;
    }

    return exit_code == STILL_ACTIVE;

#else

    if (id_ > static_cast<Id>(std::numeric_limits<pid_t>::max()))
    {
      return false;
    }

    errno = 0;

    const int result =
        ::kill(static_cast<pid_t>(id_), 0);

    if (result == 0)
    {
      return true;
    }

    return errno == EPERM;

#endif
  }

  NativeProcess::Id NativeProcess::id() const noexcept
  {
    return id_;
  }

} // namespace softadastra
