/**
 *
 *  @file remote_control_test.cpp
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

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <thread>

#include "control/ControlServer.hpp"
#include "control/RemoteAccessConfig.hpp"
#include "control/RemoteControlClient.hpp"
#include "control/RemoteControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"
namespace
{
  bool wait_for(const softadastra::RemoteControlServer &server)
  {
    for (int i = 0; i < 50; ++i) {
      if (server.listening())
        return true;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return false;
  }
  TEST(RemoteControlTest, RequiresSecretAndServesTls13Localhost)
  {
    const auto directory =
        std::filesystem::temp_directory_path() / "softadastra-remote-control-test";
    std::filesystem::remove_all(directory);
    softadastra::RemoteAccessConfig config(directory / "config");
    ASSERT_TRUE(config.save({true, "127.0.0.1", 45781}));
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService service(host, platform.process_launcher());
    softadastra::ControlServer control(service);
    softadastra::RemoteControlServer remote(control, config, "correct-secret", directory);
    ASSERT_TRUE(remote.apply());
    ASSERT_TRUE(std::filesystem::exists(directory / "remote-cert.pem"));
    ASSERT_TRUE(wait_for(remote));
    softadastra::RemoteControlClient denied("127.0.0.1", 45781, "wrong-secret");
    EXPECT_EQ(denied.request("connectivity"), std::optional<std::string>("denied"));
    softadastra::RemoteControlClient accepted("127.0.0.1", 45781, "correct-secret");
    const auto response = accepted.request("connectivity");
    ASSERT_TRUE(response.has_value());
    EXPECT_EQ(response->substr(0, 12), "connectivity");
  EXPECT_EQ(accepted.request("access")->substr(0, 6), "access");
    EXPECT_EQ(accepted.request("register 64656d6f 736c656570 1 31"),
              std::optional<std::string>("register 1"));
    EXPECT_EQ(accepted.request("start 64656d6f")->substr(0, 9), "operation");
    EXPECT_EQ(accepted.request("status 64656d6f")->substr(0, 6), "status");
    EXPECT_EQ(accepted.request("restart 64656d6f")->substr(0, 9), "operation");
    EXPECT_EQ(accepted.request("stop 64656d6f")->substr(0, 9), "operation");
    remote.stop();
    std::filesystem::remove_all(directory);
  }
}  // namespace
