/**
 *
 *  @file main.cpp
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

#include "cli/Cli.hpp"
#include "cli/CliStyle.hpp"
#include "control/ControlClient.hpp"
#include "control/ControlServer.hpp"
#include "control/LocalControlServer.hpp"
#include "control/RemoteAccessConfig.hpp"
#include "host/Host.hpp"
#include "host/HostIdentity.hpp"
#include "host/HostLoop.hpp"
#include "host/HostObservation.hpp"
#include "host/HostProfile.hpp"
#include "host/HostService.hpp"
#include "host/HostStateFile.hpp"
#include "host/LocalReachability.hpp"
#include "host/RemoteReachability.hpp"
#include "platform/HostInstanceLock.hpp"
#include "platform/MdnsPublisher.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "platform/NativeManagedNetwork.hpp"
#include "platform/NativeNetwork.hpp"
#include "platform/NativePlatform.hpp"
#include "platform/NativeService.hpp"
#include "platform/PrivilegedLocalFirewall.hpp"
#include "software/ProjectConfig.hpp"
#include "software/ProjectIdentity.hpp"
#include "webui/WebUiServer.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#if defined(__linux__)

#include "host/LocalDns.hpp"
#include "host/LocalGatewayProcessEndpoint.hpp"
#include "platform/NativeLocalDnsDelegation.hpp"

#include <csignal>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>

#endif

#if defined(_WIN32)

#include <shellapi.h>
#include <windows.h>

#endif

namespace
{
  constexpr int host_start_attempts = 500;
  constexpr std::size_t host_info_field_width = 16;

#if defined(__linux__)
  bool prepare_box_prerequisites(const std::filesystem::path &directory)
  {
    if (::geteuid() != 0) return false;
    if (::getgrnam("softadastra") == nullptr &&
        std::system("groupadd --system softadastra") != 0) return false;
    if (::getpwnam("softadastra") == nullptr &&
        std::system("useradd --system --gid softadastra --home-dir /var/lib/softadastra --shell /usr/sbin/nologin softadastra") != 0) return false;
    const auto *account = ::getpwnam("softadastra");
    if (account == nullptr) return false;
    std::error_code error;
    std::filesystem::create_directories(directory, error);
    if (error || ::chown(directory.c_str(), account->pw_uid, account->pw_gid) != 0 ||
        ::chmod(directory.c_str(), 0750) != 0) return false;
    return true;
  }
#endif

  softadastra::HostObservation host_observation(
      const softadastra::ControlClient &client)
  {
    return softadastra::observe_host(
        client,
        softadastra::NativeDataDirectory::path());
  }

  bool box_host_blocks_user_host()
  {
#if defined(__linux__)
    const auto data_directory =
        softadastra::NativeDataDirectory::box_path();
    const softadastra::ControlClient client(
        data_directory / "control.sock");

    return !softadastra::box_allows_user_host_start(
        softadastra::observe_host(
            client,
            data_directory));
#else
    return false;
#endif
  }

  bool allow_user_host_start()
  {
    if (!box_host_blocks_user_host())
    {
      return true;
    }

    std::cerr
        << "Softadastra Box Host prevents user Host startup.\n";
    return false;
  }

  bool wait_for_host(
      const softadastra::ControlClient &client,
      softadastra::HostAvailability expected,
      int attempts)
  {
    for (int attempt = 0; attempt < attempts; ++attempt)
    {
      if (host_observation(client).state == expected)
      {
        return true;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(20));
    }

    return host_observation(client).state == expected;
  }

#if defined(__linux__)

  void open_browser(
      const std::string &url)
  {
    const pid_t child =
        fork();

    if (child != 0)
    {
      if (child > 0)
      {
        static_cast<void>(
            waitpid(
                child,
                nullptr,
                0));
      }

      return;
    }

    if (fork() == 0)
    {
      execlp(
          "xdg-open",
          "xdg-open",
          url.c_str(),
          nullptr);
    }

    _exit(0);
  }

#elif defined(_WIN32)

  void open_browser(
      const std::string &url)
  {
    static_cast<void>(
        ShellExecuteA(
            nullptr,
            "open",
            url.c_str(),
            nullptr,
            nullptr,
            SW_SHOWNORMAL));
  }

#else

  void open_browser(
      const std::string &)
  {
  }

#endif

#if defined(_WIN32)

  std::atomic_bool windows_stop{false};

  BOOL WINAPI console_control(
      DWORD value)
  {
    if (value == CTRL_C_EVENT ||
        value == CTRL_BREAK_EVENT ||
        value == CTRL_CLOSE_EVENT)
    {
      windows_stop = true;
      return TRUE;
    }

    return FALSE;
  }

#endif

#if defined(__linux__)

  bool block_host_signals()
  {
    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    return pthread_sigmask(
               SIG_BLOCK,
               &signals,
               nullptr) == 0;
  }

#endif

#if defined(__linux__)

  void print_host_started(
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings,
      const softadastra::MdnsPublisher &mdns_publisher,
      bool mdns_available)
  {
    const auto access =
        host_service.local_access();

    const std::string ipv4 =
        host_service.primary_ipv4();

    std::cout
        << "Softadastra Host is running\n"
        << "hostname: "
        << (access.host_name.empty()
                ? "unavailable"
                : access.host_name)
        << "\nlocal control: ready\n"
        << "network: "
        << (host_service.connectivity_available()
                ? "available"
                : "unavailable")
        << "\nipv4: "
        << (ipv4.empty()
                ? "unavailable"
                : ipv4)
        << "\nlocal name: "
        << (mdns_available
                ? mdns_publisher.name()
                : "unavailable (mDNS unavailable)")
        << "\nremote access: "
        << (remote_settings.enabled
                ? "enabled"
                : "disabled")
        << "\n\nPress Ctrl+C to stop.\n"
        << std::flush;
  }

  int run_host(
      softadastra::HostLoop &host_loop,
      const softadastra::HostService &host_service,
      const softadastra::RemoteAccessSettings &remote_settings,
      softadastra::MdnsPublisher &mdns_publisher)
  {
    sigset_t signals;

    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    if (pthread_sigmask(
            SIG_BLOCK,
            &signals,
            nullptr) != 0)
    {
      return 1;
    }

    std::atomic_bool completed{false};

    std::thread thread(
        [&host_loop, &completed]()
        {
          completed = host_loop.run();
        });

    for (int attempt = 0;
         attempt < 1000;
         ++attempt)
    {
      if (host_loop.is_running())
      {
        const bool mdns_available =
            mdns_publisher.start(
                host_service.primary_ipv4());

        print_host_started(
            host_service,
            remote_settings,
            mdns_publisher,
            mdns_available);

        break;
      }

      if (completed)
      {
        thread.join();
        return 1;
      }

      std::this_thread::sleep_for(
          std::chrono::milliseconds(1));
    }

    if (!host_loop.is_running())
    {
      host_loop.request_stop();
      thread.join();

      return 1;
    }

    int signal = 0;

    while (!completed)
    {
      const timespec timeout{
          0,
          100000000};

      const int received =
          sigtimedwait(
              &signals,
              nullptr,
              &timeout);

      if (received == SIGINT ||
          received == SIGTERM)
      {
        signal = received;
        break;
      }
    }

    if (signal != 0)
    {
      std::cout
          << "Stopping Softadastra Host...\n"
          << std::flush;
    }

    host_loop.request_stop();
    thread.join();

    mdns_publisher.stop();

    if (completed)
    {
      std::cout
          << "Softadastra Host stopped.\n"
          << std::flush;

      return 0;
    }

    return 1;
  }

#else

  int run_host(
      softadastra::HostLoop &host_loop)
  {
#if defined(_WIN32)

    windows_stop = false;

    const bool console_handler_installed =
        ::SetConsoleCtrlHandler(
            console_control,
            TRUE) != FALSE;

    std::atomic_bool completed{false};

    std::thread thread(
        [&]
        {
          completed =
              host_loop.run();
        });

    while (!completed &&
           !windows_stop)
    {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(50));
    }

    if (windows_stop)
    {
      host_loop.request_stop();
    }

    thread.join();

    if (console_handler_installed)
    {
      ::SetConsoleCtrlHandler(
          console_control,
          FALSE);
    }

    return completed
               ? 0
               : 1;

#else

    const bool completed =
        host_loop.run();

    return completed
               ? 0
               : 1;

#endif
  }

#endif

  bool start_host_automatically(
      const char *executable)
  {
#if defined(__linux__)

    char path[4096]{};

    const ssize_t length =
        ::readlink(
            "/proc/self/exe",
            path,
            sizeof(path) - 1);

    const char *host_executable =
        length > 0
            ? path
            : executable;

    const pid_t child =
        ::fork();

    if (child < 0)
    {
      return false;
    }

    if (child == 0)
    {
      static_cast<void>(
          ::setsid());

      const int null =
          ::open(
              "/dev/null",
              O_RDWR);

      if (null >= 0)
      {
        static_cast<void>(
            ::dup2(
                null,
                STDIN_FILENO));

        static_cast<void>(
            ::dup2(
                null,
                STDOUT_FILENO));

        static_cast<void>(
            ::dup2(
                null,
                STDERR_FILENO));

        if (null > STDERR_FILENO)
        {
          ::close(null);
        }
      }

      ::execl(
          host_executable,
          host_executable,
          "host",
          nullptr);

      ::_exit(127);
    }

    return true;

#else

#if defined(_WIN32)

    std::wstring command =
        L"\"";

    for (const char character :
         std::string(executable))
    {
      command +=
          static_cast<wchar_t>(
              static_cast<unsigned char>(
                  character));
    }

    command +=
        L"\" host";

    STARTUPINFOW startup{};
    startup.cb =
        sizeof(startup);

    PROCESS_INFORMATION process{};

    const bool started =
        ::CreateProcessW(
            nullptr,
            command.data(),
            nullptr,
            nullptr,
            FALSE,
            DETACHED_PROCESS |
                CREATE_NEW_PROCESS_GROUP,
            nullptr,
            nullptr,
            &startup,
            &process) != FALSE;

    if (started)
    {
      ::CloseHandle(
          process.hThread);

      ::CloseHandle(
          process.hProcess);
    }

    return started;

#else

    static_cast<void>(
        executable);

    return false;

#endif

#endif
  }

} // namespace

int main(
    int argc,
    char *argv[])
{
  if (!softadastra::NativePlatform::host_supported())
  {
    std::cerr
        << softadastra::NativePlatform::host_support_diagnostic()
        << '\n';

    return 1;
  }

  if (argc == 2 &&
      std::string(argv[1]) == "host")
  {
#if defined(__linux__)

    if (!block_host_signals())
    {
      return 1;
    }

#endif

    if (!allow_user_host_start())
    {
      return 1;
    }

    const auto data_directory =
        softadastra::NativeDataDirectory::path();

    if (!softadastra::NativeDataDirectory::ensure_exists())
    {
      std::cerr
          << "failed to initialize Host data directory\n";

      return 1;
    }

    softadastra::HostInstanceLock instance_lock;

    if (!instance_lock.acquire(
            data_directory))
    {
      std::cerr
          << "Softadastra Host is already running\n";

      return 1;
    }

    softadastra::HostIdentity identity(
        data_directory / "identity");

    if (!identity.load_or_create())
    {
      std::cerr
          << "failed to load Host identity\n";

      return 1;
    }

    softadastra::HostProfileStore profile_store(
        data_directory / "host-profile");

    if (!profile_store.load(
            identity.id()))
    {
      std::cerr
          << "failed to load Host profile\n";

      return 1;
    }

    softadastra::NativePlatform platform;

    softadastra::Host host(
        platform);

    softadastra::HostService host_service(
        host,
        platform.process_launcher());

    softadastra::HostStateFile state_file(
        data_directory / "host-state");

    softadastra::ControlServer control_server(
        host_service);

    softadastra::RemoteAccessConfig remote_config(
        data_directory / "remote-access");

    softadastra::RemoteAccessSettings remote_settings;

    std::error_code remote_access_error;
    const bool remote_access_exists = std::filesystem::exists(
        data_directory / "remote-access", remote_access_error);
    if (remote_access_error ||
        (!remote_access_exists && !remote_config.save(remote_settings)) ||
        (remote_access_exists && !remote_config.load(remote_settings)))
    {
      std::cerr
          << (remote_access_exists
                  ? "invalid remote access configuration\n"
                  : "failed to initialize remote access configuration\n");

      return 1;
    }

    softadastra::RemoteReachability remote_reachability(
        host_service);

    if (remote_settings.enabled)
    {
      remote_reachability.configure(
          {remote_settings.address,
           remote_settings.port});
    }

    softadastra::LocalControlServer local_control_server(
        control_server,
        data_directory / "control.sock",
        &remote_config,
        &remote_reachability);

#if defined(__linux__)

    softadastra::LocalDns local_dns;

    softadastra::NativeLocalDnsDelegation
        local_dns_delegation;

    const auto gateway_executable =
        std::filesystem::absolute(
            argv[0])
            .parent_path() /
        "softadastra-gateway";

    softadastra::LocalGatewayProcessEndpoint
        gateway_endpoint(
            platform.process_launcher(),
            gateway_executable,
            data_directory / "control.sock");

    softadastra::LocalReachability
        local_reachability(
            platform.managed_network(),
            local_dns_delegation,
            local_dns,
            gateway_endpoint,
            80);

    host_service.set_local_reachability(
        &local_reachability);

#endif

    softadastra::HostLoop host_loop(
        host_service,
        state_file,
        std::chrono::seconds(1),
        &local_control_server,
        &profile_store
#if defined(__linux__)
        ,
        &local_reachability
#endif
    );

    local_control_server.set_state_persistence_handler(
        [&state_file, &host]()
        {
          return state_file.save(host.state());
        });

    local_control_server.set_shutdown_handler(
        [&host_loop]()
        {
          host_loop.request_stop();
        });

#if defined(__linux__)

    softadastra::MdnsPublisher mdns_publisher(
        identity.id());

    return run_host(
        host_loop,
        host_service,
        remote_settings,
        mdns_publisher);

#else

    return run_host(
        host_loop);

#endif
  }

  softadastra::ControlClient control_client(
      softadastra::NativeDataDirectory::path() /
      "control.sock");

  if (argc >= 2 &&
      std::string(argv[1]) == "ui")
  {
    std::uint16_t port = 0;

    if (argc == 4 &&
        std::string(argv[2]) == "--port")
    {
      try
      {
        const unsigned long parsed =
            std::stoul(argv[3]);

        if (parsed == 0 ||
            parsed > 65535)
        {
          throw std::out_of_range(
              "port");
        }

        port =
            static_cast<std::uint16_t>(
                parsed);
      }
      catch (const std::exception &)
      {
        std::cerr
            << "invalid UI port\n";

        return 2;
      }
    }
    else if (argc != 2)
    {
      std::ostream &out = std::cerr;

      out << "Usage:\n";
      out << "  softadastra ui [options]\n\n";

      out << "Open the local Softadastra web interface.\n\n";

      out << "Options:\n";
      out << "  --port <port>    Use a specific local port\n";

      return 2;
    }

    if (!host_observation(control_client).available())
    {
      if (host_observation(control_client).state !=
          softadastra::HostAvailability::Stopped)
      {
        std::cerr << "Softadastra Host is unavailable\n";
        return 1;
      }

      if (!allow_user_host_start())
      {
        return 1;
      }

      if (!start_host_automatically(
              argv[0]))
      {
        std::cerr
            << "failed to start Softadastra Host\n";

        return 1;
      }

      static_cast<void>(
          wait_for_host(
              control_client,
              softadastra::HostAvailability::Running,
              host_start_attempts));
    }

    if (!host_observation(control_client).available())
    {
      std::cerr
          << "Softadastra Host did not become available\n";

      return 1;
    }

    softadastra::WebUiServer ui(
        control_client);

    if (!ui.start(port))
    {
      std::cerr
          << "failed to start Softadastra UI on 127.0.0.1\n";

      return 1;
    }

    const std::string url =
        "http://127.0.0.1:" +
        std::to_string(ui.port());

    std::cout
        << "Softadastra UI: "
        << url
        << '\n';

    open_browser(url);

    for (;;)
    {
      std::this_thread::sleep_for(
          std::chrono::seconds(1));
    }
  }

  if (argc >= 2 &&
      std::string(argv[1]) == "box")
  {
    const auto data_directory =
        softadastra::NativeDataDirectory::box_path();

    std::error_code data_directory_error;
    std::filesystem::create_directories(
        data_directory,
        data_directory_error);

    if (data_directory_error ||
        !std::filesystem::is_directory(
            data_directory,
            data_directory_error) ||
        data_directory_error)
    {
      std::cerr
          << "failed to initialize Host data directory\n";

      return 1;
    }

    softadastra::ControlClient box_control_client(
        data_directory / "control.sock");

    softadastra::HostIdentity identity(
        data_directory / "identity");

    if (!identity.load_or_create())
    {
      std::cerr
          << "failed to load Host identity\n";

      return 1;
    }

    softadastra::HostProfileStore profile_store(
        data_directory / "host-profile");

    if (!profile_store.load(
            identity.id()))
    {
      std::cerr
          << "failed to load Host profile\n";

      return 1;
    }

    const std::string action =
        argc >= 3
            ? argv[2]
            : "";

    if (action == "provision" &&
        argc == 3)
    {
#if defined(__linux__)
      if (!prepare_box_prerequisites(data_directory))
      {
        std::cerr << "Box provisioning requires root and could not prepare its system account\n";
        return 1;
      }
#endif
      if (!profile_store.provision_box(
              identity.id()))
      {
        std::cerr
            << "failed to provision Softadastra Box\n";

        return 1;
      }

      softadastra::NativePlatform platform;
#if defined(__linux__)
      auto &service = static_cast<softadastra::NativeService &>(platform.service());
      if (!service.is_installed() || !service.enable_auto_start() || !service.start())
      {
        std::cerr << "failed to enable and start Softadastra Box service\n";
        return 1;
      }
#endif
      std::cout
          << "Softadastra Box provisioned.\n"
          << "Host ID: "
          << identity.id()
          << '\n';

      return 0;
    }

    if (action == "unprovision" &&
        argc == 3)
    {
      if (!profile_store.unprovision())
      {
        std::cerr
            << "failed to unprovision Softadastra Box\n";

        return 1;
      }

      std::cout
          << "Softadastra Box unprovisioned.\n";

      return 0;
    }

    if (action == "status" &&
        argc == 3)
    {
      const auto observation =
          softadastra::observe_host(
              box_control_client,
              data_directory);
      const bool running = observation.available();

      const auto network =
          running
              ? box_control_client
                    .managed_network_status()
                    .value_or(
                        softadastra::ManagedNetworkStatus{})
              : softadastra::ManagedNetworkStatus{};

      const auto reachability =
          running
              ? box_control_client
                    .local_reachability_state()
                    .value_or(
                        softadastra::LocalReachabilityState::
                            Unavailable)
              : softadastra::LocalReachabilityState::
                    Unavailable;

      const auto state =
          softadastra::box_state(
              profile_store.profile(),
              running,
              network,
              reachability);

      if (state ==
          softadastra::BoxState::NotProvisioned)
      {
        std::cout
            << "Not provisioned\n";

        return 0;
      }

#if defined(__linux__)
      softadastra::NativePlatform box_platform;
      if (::getpwnam("softadastra") == nullptr ||
          !static_cast<softadastra::NativeService &>(box_platform.service()).is_installed())
      {
        std::cout << "Box:              incomplete\n"
                  << "Reason:           system account or service is missing\n"
                  << "Repair:          softadastra box provision\n";
        return 0;
      }
#endif

      std::cout
          << "Box:              provisioned\n"
          << "Host:             "
          << softadastra::host_availability_name(
                 observation.state)
          << '\n';

      if (state ==
          softadastra::BoxState::Stopped)
      {
        std::cout
            << "State:            stopped\n";

        return 0;
      }

      if (network.capability ==
          softadastra::ManagedNetworkCapability::Unavailable)
      {
        std::cout
            << "Managed network:  unavailable\n"
            << "State:            degraded\n"
            << "Reason:           managed network unavailable\n";

        return 0;
      }

      if (state ==
          softadastra::BoxState::Ready)
      {
        std::cout
            << "Managed network:  running\n"
            << "Local IPv4:       "
            << network.ipv4
            << "\nState:            ready\n";

        return 0;
      }

      if (network.state ==
              softadastra::ManagedNetworkState::Running &&
          reachability !=
              softadastra::LocalReachabilityState::Ready)
      {
        std::cout
            << "Managed network:  running\n"
            << "State:            degraded\n"
            << "Reason:           local reachability is not ready\n";

        return 0;
      }

      std::cout
          << "Managed network:  "
          << softadastra::managed_network_state_name(
                 network.state)
          << "\nState:            degraded\n"
          << "Reason:           "
          << (network.state ==
                      softadastra::ManagedNetworkState::Running
                  ? "managed network has no local IPv4"
                  : "managed network is not running")
          << '\n';

      return 0;
    }

    std::ostream &out = std::cerr;

    out << "Usage:\n";
    out << "  softadastra box <command>\n\n";

    out << "Manage this Host as a Softadastra Box.\n\n";

    out << "Commands:\n";
    out << "  provision      Provision this Host as a Box\n";
    out << "  status         Show Box status\n";
    out << "  unprovision    Remove Box provisioning\n";

    return 2;
  }

  if (argc >= 3 &&
      std::string(argv[1]) == "host")
  {
    const std::string action(
        argv[2]);

    if (action == "-h" ||
        action == "--help")
    {
      std::ostream &out = std::cout;

      out << "Usage:\n";
      out << "  softadastra host\n";
      out << "  softadastra host <command>\n\n";

      out << "Run and manage the Softadastra Host.\n\n";

      out << "Commands:\n";
      out << "  start       Start the Host\n";
      out << "  stop        Stop the Host\n";
      out << "  restart     Restart the Host\n";
      out << "  status      Show Host status\n";
      out << "  info        Show Host information\n\n";

      out << "Without a command, the Host runs in the foreground.\n\n";

      out << "Options:\n";
      out << "  -h, --help  Show this help\n";

      return 0;
    }

    if (action == "status")
    {
      const auto observation =
          host_observation(control_client);

      std::cout
          << softadastra::cli::style::field(
                 "Host:",
                 softadastra::host_availability_name(
                     observation.state),
                 6)
          << '\n';

      return 0;
    }

    if (action == "info")
    {
      softadastra::HostIdentity identity(
          softadastra::NativeDataDirectory::path() /
          "identity");

      static_cast<void>(
          identity.load_or_create());

      softadastra::HostProfileStore profile_store(
          softadastra::NativeDataDirectory::path() /
          "host-profile");

      static_cast<void>(
          profile_store.load(
              identity.id()));

      const auto observation =
          host_observation(control_client);

      if (!observation.available())
      {
        std::cout
            << softadastra::cli::style::field(
                   "State:",
                   softadastra::host_availability_name(
                       observation.state),
                   host_info_field_width)
            << '\n'
            << softadastra::cli::style::field(
                   "Profile:",
                   softadastra::host_profile_name(
                       profile_store.profile()),
                   host_info_field_width)
            << '\n';

        return 0;
      }

      const auto access =
          control_client.local_access();

      softadastra::MdnsPublisher local_name(
          identity.id());

      softadastra::RemoteAccessConfig remote(
          softadastra::NativeDataDirectory::path() /
          "remote-access");

      softadastra::RemoteAccessSettings remote_settings;

      static_cast<void>(
          remote.load(
              remote_settings));

      const auto pid =
          control_client.request(
              "host-pid");

      const auto entries =
          control_client.software();

      std::size_t running = 0;
      std::size_t stopped = 0;
      std::size_t starting = 0;
      std::size_t failed = 0;

      for (const auto &entry : entries)
      {
        if (entry.state() ==
            softadastra::SoftwareState::Running)
        {
          ++running;
        }
        else if (entry.state() ==
                 softadastra::SoftwareState::Stopped)
        {
          ++stopped;
        }
        else if (entry.state() ==
                 softadastra::SoftwareState::Starting)
        {
          ++starting;
        }
        else if (entry.state() ==
                 softadastra::SoftwareState::Failed)
        {
          ++failed;
        }
      }

      std::cout
          << softadastra::cli::style::field("State:", "running", host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Profile:", softadastra::host_profile_name(profile_store.profile()), host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Hostname:", access ? access->host_name : "-", host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Host ID:", identity.id(), host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("PID:", pid ? pid->substr(9) : "-", host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("IPv4:", access && !access->primary_ipv4.empty() ? access->primary_ipv4 : "-", host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Local name:", identity.id().empty() ? "-" : local_name.name(), host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Connectivity:", control_client.connectivity_available() ? "available" : "unavailable", host_info_field_width)
          << '\n'
          << softadastra::cli::style::field("Remote access:", remote_settings.enabled ? "enabled" : "disabled", host_info_field_width)
          << "\n\nSoftware:\n"
          << "  total:    "
          << entries.size()
          << "\n  running:  "
          << running
          << "\n  stopped:  "
          << stopped
          << "\n  starting: "
          << starting
          << "\n  failed:   "
          << failed
          << '\n';

      return 0;
    }

    if (action == "stop")
    {
      const auto observation =
          host_observation(control_client);

      if (observation.state ==
          softadastra::HostAvailability::Stopped)
      {
        std::cout
            << softadastra::cli::style::warning(
                   "already stopped:")
            << " host\n";

        return 0;
      }

      if (!observation.available())
      {
        std::cerr << "Softadastra Host is unavailable.\n";
        return 1;
      }

      if (control_client.request("shutdown") != "shutdown 1")
      {
        std::cerr
            << "Failed to stop Softadastra Host.\n";

        return 1;
      }

      if (!wait_for_host(
              control_client,
              softadastra::HostAvailability::Stopped,
              250))
      {
        std::cerr
            << "Softadastra Host did not stop.\n";

        return 1;
      }

      std::cout
          << softadastra::cli::style::success("stopped:")
          << " host\n";

      return 0;
    }

    if (action == "start" ||
        action == "restart")
    {
      const auto initial =
          host_observation(control_client);

      if (initial.state == softadastra::HostAvailability::Unavailable ||
          initial.state == softadastra::HostAvailability::Unknown)
      {
        std::cerr << "Softadastra Host is unavailable.\n";
        return 1;
      }

      const bool was_running = initial.available();

      if (!was_running &&
          !allow_user_host_start())
      {
        return 1;
      }

      if (action == "restart" &&
          was_running)
      {
        if (control_client.request("shutdown") != "shutdown 1")
        {
          std::cerr << "Failed to stop Softadastra Host.\n";
          return 1;
        }

        if (!wait_for_host(
                control_client,
                softadastra::HostAvailability::Stopped,
                250))
        {
          std::cerr << "Softadastra Host did not stop.\n";
          return 1;
        }
      }

      if (!host_observation(control_client).available())
      {
        if (!start_host_automatically(
                argv[0]))
        {
          return 1;
        }

        if (!wait_for_host(
                control_client,
                softadastra::HostAvailability::Running,
                host_start_attempts))
        {
          std::cerr << "Softadastra Host did not become available\n";
          return 1;
        }
      }

      std::cout
          << (action == "restart"
                  ? softadastra::cli::style::success("restarted:")
                  : (was_running
                         ? softadastra::cli::style::warning("already running:")
                         : softadastra::cli::style::success("started:")))
          << " host\n";

      return 0;
    }

    std::ostream &out = std::cerr;

    out << "Unknown host action: "
        << action
        << "\n\n";

    out << "Usage:\n";
    out << "  softadastra host\n";
    out << "  softadastra host <command>\n\n";

    out << "Run and manage the Softadastra Host.\n\n";

    out << "Commands:\n";
    out << "  start       Start the Host\n";
    out << "  stop        Stop the Host\n";
    out << "  restart     Restart the Host\n";
    out << "  status      Show Host status\n";
    out << "  info        Show Host information\n\n";

    out << "Without a command, the Host runs in the foreground.\n";

    return 2;
  }

  const std::string cli_command =
      argc > 1
          ? argv[1]
          : "";

  const bool command_help =
      argc == 3 &&
      (std::string(argv[2]) == "-h" ||
       std::string(argv[2]) == "--help");

  const bool legacy_migration =
      cli_command == "init" &&
      softadastra::ProjectIdentity::find(
          std::filesystem::current_path())
          .has_value();

  const bool needs_host =
      cli_command != "" &&
      cli_command != "host" &&
      cli_command != "help" &&
      cli_command != "-h" &&
      cli_command != "--help" &&
      cli_command != "box" &&
      cli_command != "network" &&
      cli_command != "access" &&
      (cli_command != "init" ||
       legacy_migration) &&
      !command_help;

  if (needs_host &&
      !host_observation(control_client).available())
  {
    if (host_observation(control_client).state !=
        softadastra::HostAvailability::Stopped)
    {
      std::cerr << "Softadastra Host is unavailable\n";
      return 1;
    }

    if (!allow_user_host_start())
    {
      return 1;
    }

    if (!start_host_automatically(
            argv[0]))
    {
      std::cerr
          << "failed to start Softadastra Host\n";

      return 1;
    }

    static_cast<void>(
        wait_for_host(
            control_client,
            softadastra::HostAvailability::Running,
            host_start_attempts));

    if (!host_observation(control_client).available())
    {
      std::cerr
          << "Softadastra Host did not become available\n";

      return 1;
    }
  }

  softadastra::NativeNetwork network;
  softadastra::NativeManagedNetwork managed_network;
  softadastra::NativePrivilegedProcessRunner privileged_process_runner;
  softadastra::PrivilegedLocalFirewall local_firewall(privileged_process_runner);

  softadastra::Cli cli(
      control_client,
      network,
      managed_network,
      local_firewall);

  std::vector<const char *> arguments;

  arguments.reserve(
      static_cast<std::size_t>(argc));

  for (int index = 0;
       index < argc;
       ++index)
  {
    arguments.push_back(
        argv[index]);
  }

  return cli.run(
      argc,
      arguments.data());
}
