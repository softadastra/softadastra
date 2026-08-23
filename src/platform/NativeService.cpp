/**
 *
 *  @file NativeService.cpp
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

#include "platform/NativeService.hpp"

#if defined(_WIN32)

#include <windows.h>

#elif defined(__linux__)

#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace
{
#if defined(_WIN32)

  constexpr wchar_t service_name[] = L"softadastra";

  SC_HANDLE open_service(DWORD access) noexcept
  {
    SC_HANDLE manager = OpenSCManagerW(
        nullptr,
        nullptr,
        SC_MANAGER_CONNECT);

    if (manager == nullptr)
    {
      return nullptr;
    }

    SC_HANDLE service = OpenServiceW(
        manager,
        service_name,
        access);

    CloseServiceHandle(manager);

    return service;
  }

  bool service_running(SC_HANDLE service) noexcept
  {
    SERVICE_STATUS_PROCESS status{};
    DWORD bytes_needed = 0;

    if (QueryServiceStatusEx(
            service,
            SC_STATUS_PROCESS_INFO,
            reinterpret_cast<LPBYTE>(&status),
            sizeof(status),
            &bytes_needed) == FALSE)
    {
      return false;
    }

    return status.dwCurrentState == SERVICE_RUNNING;
  }

#elif defined(__linux__)

  int run_systemctl(
      const char *operation,
      bool quiet = false) noexcept
  {
    const pid_t child = ::fork();

    if (child < 0)
    {
      return -1;
    }

    if (child == 0)
    {
      if (quiet)
      {
        ::execlp(
            "systemctl",
            "systemctl",
            operation,
            "--quiet",
            "softadastra.service",
            static_cast<char *>(nullptr));
      }
      else
      {
        ::execlp(
            "systemctl",
            "systemctl",
            operation,
            "softadastra.service",
            static_cast<char *>(nullptr));
      }

      ::_exit(127);
    }

    int status = 0;

    for (;;)
    {
      const pid_t result =
          ::waitpid(child, &status, 0);

      if (result == child)
      {
        break;
      }

      if (result < 0 && errno == EINTR)
      {
        continue;
      }

      return -1;
    }

    if (!WIFEXITED(status))
    {
      return -1;
    }

    return WEXITSTATUS(status);
  }

#endif

} // namespace

namespace softadastra
{

  bool NativeService::start()
  {
#if defined(_WIN32)

    SC_HANDLE service = open_service(
        SERVICE_START | SERVICE_QUERY_STATUS);

    if (service == nullptr)
    {
      return false;
    }

    if (service_running(service))
    {
      CloseServiceHandle(service);
      return true;
    }

    const BOOL result =
        StartServiceW(service, 0, nullptr);

    if (result == FALSE &&
        GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
    {
      CloseServiceHandle(service);
      return false;
    }

    CloseServiceHandle(service);

    return true;

#elif defined(__linux__)

    if (is_running())
    {
      return true;
    }

    return run_systemctl("start") == 0;

#else

    return false;

#endif
  }

  bool NativeService::stop()
  {
#if defined(_WIN32)

    SC_HANDLE service = open_service(
        SERVICE_STOP | SERVICE_QUERY_STATUS);

    if (service == nullptr)
    {
      return false;
    }

    if (!service_running(service))
    {
      CloseServiceHandle(service);
      return true;
    }

    SERVICE_STATUS status{};

    const BOOL result =
        ControlService(
            service,
            SERVICE_CONTROL_STOP,
            &status);

    CloseServiceHandle(service);

    return result != FALSE;

#elif defined(__linux__)

    if (!is_running())
    {
      return true;
    }

    return run_systemctl("stop") == 0;

#else

    return false;

#endif
  }

  bool NativeService::is_running() const noexcept
  {
#if defined(_WIN32)

    SC_HANDLE service = open_service(
        SERVICE_QUERY_STATUS);

    if (service == nullptr)
    {
      return false;
    }

    const bool running =
        service_running(service);

    CloseServiceHandle(service);

    return running;

#elif defined(__linux__)

    return run_systemctl("is-active", true) == 0;

#else

    return false;

#endif
  }

} // namespace softadastra
