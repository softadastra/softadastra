/**
 *
 *  @file host_runtime_test.cpp
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

#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/ProcessSpec.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <gtest/gtest.h>

namespace
{

  softadastra::ProcessSpec make_long_running_process()
  {
#if defined(_WIN32)

    return softadastra::ProcessSpec(
        "cmd.exe",
        {
            "/C",
            "ping -n 30 127.0.0.1 >NUL",
        });

#else

    return softadastra::ProcessSpec(
        "sh",
        {
            "-c",
            "sleep 30",
        });

#endif
  }

  TEST(HostRuntimeTest, RunsSoftwareThroughCompleteHostControlPath)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer control_server(host_service);
    softadastra::ControlClient control_client(control_server);

    const softadastra::SoftwareId id("integration-test");

    ASSERT_TRUE(
        control_client.register_software(
            id,
            make_long_running_process()));

    ASSERT_TRUE(control_client.software_state(id).has_value());

    EXPECT_EQ(
        control_client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);

    ASSERT_TRUE(control_client.start_software(id));

    ASSERT_TRUE(control_client.software_state(id).has_value());

    EXPECT_EQ(
        control_client.software_state(id).value(),
        softadastra::SoftwareState::Running);

    ASSERT_TRUE(control_client.stop_software(id));

    ASSERT_TRUE(control_client.software_state(id).has_value());

    EXPECT_EQ(
        control_client.software_state(id).value(),
        softadastra::SoftwareState::Stopped);
  }

  TEST(HostRuntimeTest, RejectsSoftwareThatCannotBeLaunched)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer control_server(host_service);
    softadastra::ControlClient control_client(control_server);

    const softadastra::SoftwareId id("missing-software");

    ASSERT_TRUE(
        control_client.register_software(
            id,
            softadastra::ProcessSpec(
                "softadastra-executable-that-does-not-exist")));

    EXPECT_FALSE(control_client.start_software(id));

    ASSERT_TRUE(control_client.software_state(id).has_value());

    EXPECT_EQ(
        control_client.software_state(id).value(),
        softadastra::SoftwareState::Failed);
  }

  TEST(HostRuntimeTest, ReportsNativeConnectivityConsistently)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::ControlServer control_server(host_service);
    const softadastra::ControlClient control_client(control_server);

    if (control_client.connected())
    {
      EXPECT_TRUE(control_client.connectivity_available());
    }
  }

} // namespace
