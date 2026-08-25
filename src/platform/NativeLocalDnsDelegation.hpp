#ifndef SOFTADASTRA_PLATFORM_NATIVE_LOCAL_DNS_DELEGATION_HPP
#define SOFTADASTRA_PLATFORM_NATIVE_LOCAL_DNS_DELEGATION_HPP
#include "platform/LocalDnsDelegation.hpp"
#include <filesystem>
namespace softadastra { class NativeLocalDnsDelegation final : public LocalDnsDelegation { public: explicit NativeLocalDnsDelegation(std::filesystem::path directory = "/etc/NetworkManager/dnsmasq-shared.d") : directory_(std::move(directory)) {} [[nodiscard]] LocalDnsDelegationState status() const override; private: std::filesystem::path directory_; }; }
#endif
