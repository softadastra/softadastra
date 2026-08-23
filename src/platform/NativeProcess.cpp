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

#include <utility>
#include <vix/process/Status.hpp>
#include <vix/process/Terminate.hpp>
#include <vix/process/Wait.hpp>

namespace softadastra
{

  NativeProcess::NativeProcess(vix::process::Child child) noexcept
      : child_(std::move(child))
  {
  }

  bool NativeProcess::stop()
  {
    if (!child_.valid())
    {
      return false;
    }

    if (!is_running())
    {
      return exit_code().has_value();
    }

    const auto error = vix::process::terminate(child_);

    if (error.has_error())
    {
      return false;
    }

    const auto result = vix::process::wait(child_);

    if (!result)
    {
      return false;
    }

    exit_code_ = result.value();

    return true;
  }

  bool NativeProcess::is_running() const noexcept
  {
    if (!child_.valid())
    {
      return false;
    }

    try
    {
      const auto result = vix::process::status(child_);

      if (!result)
      {
        return false;
      }

      return result.value();
    }
    catch (...)
    {
      return false;
    }
  }

  std::optional<int> NativeProcess::exit_code() noexcept
  {
    if (exit_code_.has_value())
    {
      return exit_code_;
    }

    if (!child_.valid())
    {
      return std::nullopt;
    }

    try
    {
      const auto status = vix::process::status(child_);

      if (!status)
      {
        return std::nullopt;
      }

      if (status.value())
      {
        return std::nullopt;
      }

      const auto result = vix::process::wait(child_);

      if (!result)
      {
        return std::nullopt;
      }

      exit_code_ = result.value();

      return exit_code_;
    }
    catch (...)
    {
      return std::nullopt;
    }
  }

  NativeProcess::Id NativeProcess::id() const noexcept
  {
    return child_.id();
  }

} // namespace softadastra
