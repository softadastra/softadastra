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
#include <vix/process/Process.hpp>

#include <memory>

namespace softadastra
{
  std::unique_ptr<Process> NativeProcessLauncher::launch(
      const ProcessSpec &spec)
  {
    if (spec.executable().empty())
    {
      return nullptr;
    }

    vix::process::Command command(spec.executable());

    command.args(spec.arguments());
    command.search_in_path(true);
    command.detach(false);

    auto result = vix::process::spawn(std::move(command));

    if (!result)
    {
      return nullptr;
    }

    const vix::process::Child &child = result.value();

    if (!child.valid())
    {
      return nullptr;
    }

    return std::make_unique<NativeProcess>(
        static_cast<NativeProcess::Id>(child.id()));
  }

} // namespace softadastra
