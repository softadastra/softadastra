#include <iostream>

#include <softadastra/core/types/Result.hpp>
#include <softadastra/core/errors/Error.hpp>
#include <softadastra/core/errors/ErrorCode.hpp>
#include <softadastra/core/errors/Severity.hpp>

using namespace softadastra::core;

types::Result<int, errors::Error> divide(int a, int b)
{
  if (b == 0)
  {
    return types::Result<int, errors::Error>::err(
        errors::Error(
            errors::ErrorCode::InvalidArgument,
            errors::Severity::Error,
            "division by zero"));
  }

  return types::Result<int, errors::Error>::ok(a / b);
}

int main()
{
  auto res = divide(10, 0);

  if (res.is_err())
  {
    std::cout << "Error: " << res.error().message() << "\n";
    return 1;
  }

  std::cout << "Result: " << res.value() << "\n";
  return 0;
}
