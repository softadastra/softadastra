#ifndef SOFTADASTRA_HOST_LOCAL_REACHABILITY_HPP
#define SOFTADASTRA_HOST_LOCAL_REACHABILITY_HPP
#include "platform/LocalDnsDelegation.hpp"
#include "platform/ManagedNetwork.hpp"
#include <cstdint>
#include <string>
namespace softadastra {
enum class LocalReachabilityState { Unavailable, Starting, Ready, Degraded };
class LocalDnsEndpoint { public: virtual ~LocalDnsEndpoint()=default; virtual bool start(std::string address,std::uint16_t port)=0; virtual void stop() noexcept=0; };
class LocalGatewayEndpoint { public: virtual ~LocalGatewayEndpoint()=default; virtual bool start(std::string address,std::uint16_t port)=0; virtual void stop() noexcept=0; };
class LocalReachability { public: LocalReachability(ManagedNetwork &,const LocalDnsDelegation &,LocalDnsEndpoint &,LocalGatewayEndpoint &,std::uint16_t gateway_port) noexcept; [[nodiscard]] LocalReachabilityState start(); void stop() noexcept; [[nodiscard]] LocalReachabilityState state() const noexcept { return state_; } private: ManagedNetwork &network_; const LocalDnsDelegation &delegation_; LocalDnsEndpoint &dns_; LocalGatewayEndpoint &gateway_; std::uint16_t gateway_port_; LocalReachabilityState state_{LocalReachabilityState::Unavailable}; bool dns_started_{false}; bool gateway_started_{false}; }; }
#endif
