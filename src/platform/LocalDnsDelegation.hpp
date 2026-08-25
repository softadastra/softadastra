#ifndef SOFTADASTRA_PLATFORM_LOCAL_DNS_DELEGATION_HPP
#define SOFTADASTRA_PLATFORM_LOCAL_DNS_DELEGATION_HPP
#include <string>
namespace softadastra {
enum class LocalDnsDelegationState { Available, Unavailable, Misconfigured };
class LocalDnsDelegation { public: virtual ~LocalDnsDelegation() = default; [[nodiscard]] virtual LocalDnsDelegationState status() const = 0; [[nodiscard]] static std::string configuration(); };
class UnavailableLocalDnsDelegation final : public LocalDnsDelegation { public: [[nodiscard]] LocalDnsDelegationState status() const override { return LocalDnsDelegationState::Unavailable; } };
}
#endif
