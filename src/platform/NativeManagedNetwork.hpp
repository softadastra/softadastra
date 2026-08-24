#ifndef SOFTADASTRA_PLATFORM_NATIVE_MANAGED_NETWORK_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_MANAGED_NETWORK_HPP
#include "platform/ManagedNetwork.hpp"
#include "platform/Network.hpp"
#include <string>
#include <vector>
namespace softadastra {
struct NmcliResult { int code{-1}; std::string output; };
class NmcliRunner { public: virtual ~NmcliRunner() = default; [[nodiscard]] virtual NmcliResult run(const std::vector<std::string> &arguments) = 0; };
class NativeManagedNetwork final : public ManagedNetwork { public: explicit NativeManagedNetwork(NmcliRunner *runner = nullptr, const Network *network = nullptr) noexcept : runner_(runner), network_(network) {} [[nodiscard]] ManagedNetworkStatus status() const override; [[nodiscard]] ManagedNetworkStartResult start() override; bool stop() override; private: NmcliRunner *runner_; const Network *network_; }; }
#endif
