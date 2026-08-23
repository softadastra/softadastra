/**
 *
 *  @file HostLoop.cpp
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

#include "host/HostLoop.hpp"

namespace softadastra
{
  HostLoop::HostLoop(
      HostService &host_service,
      HostStateFile &state_file,
      std::chrono::milliseconds interval) noexcept
      : host_service_(host_service),
        state_file_(state_file),
        interval_(interval > std::chrono::milliseconds::zero()
                      ? interval
                      : std::chrono::milliseconds(1))
  {
  }

  bool HostLoop::run()
  {
    if (state_file_.exists() &&
        !state_file_.load(host_service_.host().state()))
    {
      return false;
    }

    {
      std::lock_guard lock(mutex_);

      if (running_)
      {
        return false;
      }

      running_ = true;
      stop_requested_ = false;
    }

    for (;;)
    {
      std::unique_lock lock(mutex_);

      if (condition_.wait_for(
              lock,
              interval_,
              [this]()
              {
                return stop_requested_;
              }))
      {
        lock.unlock();
        const bool stopped = shutdown();
        lock.lock();
        running_ = false;
        return stopped;
      }

      lock.unlock();
      host_service_.refresh();
    }
  }

  void HostLoop::request_stop() noexcept
  {
    {
      std::lock_guard lock(mutex_);
      stop_requested_ = true;
    }

    condition_.notify_all();
  }

  bool HostLoop::is_running() const noexcept
  {
    std::lock_guard lock(mutex_);
    return running_;
  }

  bool HostLoop::shutdown()
  {
    const bool stopped = host_service_.shutdown();
    const bool saved = state_file_.save(host_service_.host().state());
    return stopped && saved;
  }

} // namespace softadastra
