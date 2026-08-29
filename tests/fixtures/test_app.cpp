#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#if defined(__linux__)
#include <csignal>

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

  return exit_code;
}
