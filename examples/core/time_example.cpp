#include <iostream>

#include <softadastra/core/Core.hpp>

using namespace softadastra::core::time;

int main()
{
  auto now = Timestamp::now();
  auto d = Duration::from_seconds(2);

  if (now.is_valid())
  {
    std::cout << "Now (ms): " << now.millis() << "\n";
  }

  std::cout << "Duration (ms): " << d.millis() << "\n";

  return 0;
}
