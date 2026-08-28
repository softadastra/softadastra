/**
 * @file local_control_windows_test.cpp
 * @brief Named Pipe transport coverage for the portable local-control contract.
 */

#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "control/LocalControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <thread>

namespace
{
  TEST(LocalControlWindowsTest, ServesTheSharedProtocolOverNamedPipes)
  {
    softadastra::NativePlatform platform;
    softadastra::Host host(platform);
    softadastra::HostService host_service(host, platform.process_launcher());
    softadastra::ControlServer control_server(host_service);
    const auto path = std::filesystem::temp_directory_path() /
                      "softadastra-local-control-windows-test";
    softadastra::LocalControlServer server(control_server, path);
    ASSERT_TRUE(server.start());

    std::atomic_bool stop{false};
    std::thread worker([&server, &stop]()
                       {
                         while (!stop)
                         {
                           static_cast<void>(server.process_pending());
                           std::this_thread::sleep_for(std::chrono::milliseconds(1));
                         }
                       });

    softadastra::ControlClient client(path);
    EXPECT_EQ(client.request("ping"), "ok");

    stop = true;
    worker.join();
    server.stop();
  }
} // namespace
