#include <iostream>

#include <softadastra/core/ids/FileId.hpp>

using namespace softadastra::core::ids;

int main()
{
  FileId id = FileId::generate();

  std::cout << "FileId: " << id.str() << "\n";

  return 0;
}
