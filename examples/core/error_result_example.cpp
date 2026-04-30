#include <iostream>
#include <softadastra/core/Core.hpp>

using namespace softadastra::core;

using IntResult = types::Result<int, errors::Error>;

IntResult divide(int a, int b)
{
  if (b == 0)
  {
    return IntResult::err(
        errors::Error::make(
            errors::ErrorCode::InvalidArgument,
            "division by zero"));
  }

  return IntResult::ok(a / b);
}

int main()
{
  auto res = divide(10, 0);

  if (res.is_err())
  {
    const auto &err = res.error();

    std::cout << "Error: " << err.message() << "\n";
    return 1;
  }

  std::cout << "Result: " << res.value() << "\n";
  return 0;
}
