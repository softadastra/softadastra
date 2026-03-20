#include <iostream>

#include <softadastra/core/time/Timestamp.hpp>
#include <softadastra/core/time/Duration.hpp>

using namespace softadastra::core::time;

int main()
{
  Timestamp now = Timestamp::now();
  Duration d = Duration::from_seconds(2);

  std::cout << "Now: " << now.value() << "\n";
  std::cout << "Duration(ms): " << d.millis() << "\n";

  return 0;
}
