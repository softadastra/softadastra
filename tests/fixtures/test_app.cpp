#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#if defined(__linux__)
#include <csignal>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
  volatile std::sig_atomic_t stopping = 0;

  void stop(int)
  {
    stopping = 1;
  }
}
#endif

int main(int argc, char *argv[])
{
  bool stay = false;
  int exit_code = 0;

#if defined(__linux__)
  int listener = -1;
#endif

  for (int index = 1; index < argc; ++index)
  {
    const std::string argument(argv[index]);

    if (argument == "--stay")
    {
      stay = true;
    }
    else if (argument == "--stdout" && index + 1 < argc)
    {
      std::cout << argv[++index] << '\n';
    }
    else if (argument == "--stderr" && index + 1 < argc)
    {
      std::cerr << argv[++index] << '\n';
    }
    else if (argument == "--exit" && index + 1 < argc)
    {
      exit_code = std::atoi(argv[++index]);
    }
#if defined(__linux__)
    else if (argument == "--listen" && index + 1 < argc)
    {
      const int port = std::atoi(argv[++index]);
      listener = ::socket(AF_INET, SOCK_STREAM, 0);
      sockaddr_in address{};
      address.sin_family = AF_INET;
      address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      address.sin_port = htons(static_cast<std::uint16_t>(port));

      if (listener < 0 ||
          ::bind(
              listener,
              reinterpret_cast<const sockaddr *>(&address),
              sizeof(address)) != 0 ||
          ::listen(listener, 1) != 0)
      {
        if (listener >= 0)
        {
          ::close(listener);
        }

        return 2;
      }
    }
#endif
    else if (argument == "--echo")
    {
      for (++index; index < argc; ++index)
      {
        std::cout << argv[index] << '\n';
      }
      break;
    }
  }

#if defined(__linux__)
  std::signal(SIGTERM, stop);
  std::signal(SIGINT, stop);
#endif

  while (stay
#if defined(__linux__)
         && stopping == 0
#endif
  )
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

#if defined(__linux__)
  if (listener >= 0)
  {
    ::close(listener);
  }
#endif

  return exit_code;
}
