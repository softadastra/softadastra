/**
 * @file local_control_windows_test.cpp
 * @brief Named Pipe transport coverage for the portable local-control contract.
 */

#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "control/LocalControlEndpoint.hpp"
#include "control/LocalControlServer.hpp"
#include "host/Host.hpp"
#include "host/HostService.hpp"
#include "platform/NativePlatform.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <array>
#include <chrono>
#include <filesystem>
#include <thread>

#if defined(_WIN32)

#include <windows.h>

#endif

#if defined(_WIN32)

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

  TEST(LocalControlWindowsTest, UsesOneEndpointForLongAndShortDirectoryNames)
  {
    const auto directory = std::filesystem::temp_directory_path() /
                           "softadastra-local-control-path-identity";
    std::filesystem::create_directories(directory);

    std::array<wchar_t, 32768> short_directory{};
    const DWORD length = ::GetShortPathNameW(
        directory.c_str(),
        short_directory.data(),
        static_cast<DWORD>(short_directory.size()));

    if (length == 0 || length >= short_directory.size())
    {
      GTEST_SKIP() << "Windows did not provide a short path for this directory";
    }

    const auto short_path =
        std::filesystem::path(std::wstring(short_directory.data(), length));

    if (short_path == directory)
    {
      GTEST_SKIP() << "Windows short-name generation is unavailable";
    }

    EXPECT_EQ(
        softadastra::local_control_pipe_name(directory / "control.sock"),
        softadastra::local_control_pipe_name(short_path / "control.sock"));

    std::filesystem::remove_all(directory);
  }
} // namespace

#endif
