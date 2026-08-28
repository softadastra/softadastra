/**
 *
 *  @file native_service_test.cpp
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
#include "platform/Service.hpp"

#include <gtest/gtest.h>

#include <string_view>
#include <type_traits>

namespace
{
  TEST(NativeServiceTest, UsesCanonicalServiceName)
  {
    EXPECT_EQ(
        softadastra::NativeService::name(),
        std::string_view("softadastra"));
  }

  TEST(NativeServiceTest, ImplementsServiceContract)
  {
    EXPECT_TRUE((
        std::is_base_of_v<
            softadastra::Service,
            softadastra::NativeService>));
  }

  TEST(NativeServiceTest, CanInspectNativeServiceState)
  {
    const softadastra::NativeService service;

    EXPECT_NO_THROW(
        static_cast<void>(service.is_running()));
  }

  TEST(NativeServiceTest, SupportsUseThroughServiceInterface)
  {
    const softadastra::NativeService native_service;
    const softadastra::Service &service = native_service;

    EXPECT_NO_THROW(
        static_cast<void>(service.is_running()));
  }

#if defined(__linux__)
  TEST(NativeServiceTest, GeneratesSystemdHostUnit)
  {
    EXPECT_EQ(
        softadastra::NativeService::unit_file_path(),
        std::filesystem::path("/etc/systemd/system/softadastra.service"));
    EXPECT_EQ(
        softadastra::NativeService::unit_file_content("/opt/softadastra/bin/softadastra"),
        "[Unit]\n"
        "Description=Softadastra Host\n"
        "After=network.target\n"
        "\n"
        "[Service]\n"
        "Type=simple\n"
        "ExecStart=\"/opt/softadastra/bin/softadastra\" host\n"
        "Restart=on-failure\n"
        "\n"
        "[Install]\n"
        "WantedBy=multi-user.target\n");
  }

  TEST(NativeServiceTest, ExposesSystemdAutoStartOperation)
  {
    using EnableAutoStart = bool (softadastra::NativeService::*)();

    EXPECT_TRUE((std::is_same_v<
                 decltype(&softadastra::NativeService::enable_auto_start),
                 EnableAutoStart>));
  }
#endif

} // namespace
