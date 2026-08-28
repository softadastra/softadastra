/**
 *
 *  @file LocalGatewayProcessEndpoint.cpp
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

#include "host/LocalGatewayProcessEndpoint.hpp"

#include <chrono>
#include <thread>
#include <utility>

namespace softadastra
{
  LocalGatewayProcessEndpoint::LocalGatewayProcessEndpoint(
      ProcessLauncher &launcher,
      std::filesystem::path executable,
      std::filesystem::path control) noexcept
      : launcher_(launcher),
        executable_(std::move(executable)),
        control_(std::move(control))
  {
  }

  bool LocalGatewayProcessEndpoint::start(
      std::string address,
      std::uint16_t port)
  {
    if (process_ &&
        process_->is_running())
    {
      return true;
    }

    process_.reset();

    auto result =
        launcher_.launch(
            ProcessSpec(
                executable_.string(),
                {"--listen",
                 address + ":" + std::to_string(port),
                 "--control",
                 control_.string()}));

    if (!result)
    {
      return false;
    }

    process_ =
        std::move(result).take_process();

    std::this_thread::sleep_for(
        std::chrono::milliseconds(20));

    if (!process_->is_running())
    {
      process_.reset();
      return false;
    }

    return true;
  }

  void LocalGatewayProcessEndpoint::stop() noexcept
  {
    if (process_)
    {
      static_cast<void>(
          process_->stop());
    }

    process_.reset();
  }

} // namespace softadastra
