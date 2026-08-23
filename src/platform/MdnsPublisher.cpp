/**
 *
 *  @file MdnsPublisher.cpp
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

#include "platform/MdnsPublisher.hpp"

#include <chrono>
#include <thread>
#include <utility>

#if defined(__linux__)

#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#endif

namespace softadastra
{
  MdnsPublisher::MdnsPublisher(std::string host_id) noexcept
      : name_("softadastra-" + host_id.substr(0, 8) + ".local")
  {
  }

  MdnsPublisher::~MdnsPublisher()
  {
    stop();
  }

  bool MdnsPublisher::available() noexcept
  {
#if defined(__linux__)
    return ::access("/usr/bin/avahi-publish-address", X_OK) == 0;
#else
    return false;
#endif
  }

  const std::string &MdnsPublisher::name() const noexcept
  {
    return name_;
  }

  bool MdnsPublisher::start(const std::string &ipv4) noexcept
  {
#if defined(__linux__)
    stop();

    if (!available() || ipv4.empty())
    {
      return false;
    }

    const pid_t child = ::fork();

    if (child < 0)
    {
      return false;
    }

    if (child == 0)
    {
      ::execl(
          "/usr/bin/avahi-publish-address",
          "avahi-publish-address",
          name_.c_str(),
          ipv4.c_str(),
          nullptr);
      ::_exit(127);
    }

    process_ = static_cast<int>(child);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    if (::waitpid(process_, nullptr, WNOHANG) == process_)
    {
      process_ = -1;
      return false;
    }

    return true;
#else
    static_cast<void>(ipv4);
    return false;
#endif
  }

  void MdnsPublisher::stop() noexcept
  {
#if defined(__linux__)
    if (process_ >= 0)
    {
      static_cast<void>(::kill(process_, SIGTERM));
      static_cast<void>(::waitpid(process_, nullptr, 0));
      process_ = -1;
    }
#endif
  }

} // namespace softadastra
