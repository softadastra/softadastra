/*
 * operation_codec.cpp
 */

#include <iostream>

#include <softadastra/store/Store.hpp>

using namespace softadastra;

int main()
{
  std::cout << "== STORE OPERATION CODEC EXAMPLE ==\n";

  auto operation = store::core::Operation::put(
      store::types::Key{"user:1"},
      store::types::Value::from_string("Gaspard"));

  auto payload =
      store::encoding::OperationEncoder::encode(operation);

  if (payload.empty())
  {
    std::cerr << "encoding failed\n";
    return 1;
  }

  auto decoded =
      store::encoding::OperationDecoder::decode(payload);

  if (!decoded.has_value())
  {
    std::cerr << "decoding failed\n";
    return 1;
  }

  std::cout << "Decoded operation type="
            << store::types::to_string(decoded->type)
            << " key="
            << decoded->key.str()
            << " value="
            << decoded->value.to_string()
            << "\n";

  return 0;
}
