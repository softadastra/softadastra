#include <iostream>

#include <softadastra/core/Core.hpp>

using namespace softadastra::core::ids;

int main()
{
  auto id = FileId::generate();

  if (id.is_valid())
  {
    std::cout << "FileId: " << id.str() << "\n";
  }

  return 0;
}
