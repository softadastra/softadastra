#include "host/LocalReachability.hpp"
#include "platform/LocalDnsConfiguration.hpp"
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#endif
namespace softadastra { LocalReachability::LocalReachability(ManagedNetwork &n,const LocalDnsDelegation &d,LocalDnsEndpoint &dns,LocalGatewayEndpoint &g,std::uint16_t p)noexcept:network_(n),delegation_(d),dns_(dns),gateway_(g),gateway_port_(p){} LocalReachabilityState LocalReachability::start(){if(state_==LocalReachabilityState::Ready)return state_;state_=LocalReachabilityState::Starting;const auto result=network_.start();if(result==ManagedNetworkStartResult::Unavailable)return state_=LocalReachabilityState::Unavailable;const auto network=network_.status();in_addr address{};if(network.capability!=ManagedNetworkCapability::Available||network.state!=ManagedNetworkState::Running)return state_=LocalReachabilityState::Unavailable;if(::inet_pton(AF_INET,network.ipv4.c_str(),&address)!=1)return state_=LocalReachabilityState::Degraded;if(delegation_.status()!=LocalDnsDelegationState::Available)return state_=LocalReachabilityState::Degraded;if(!dns_.start("127.0.0.1",local_dns_port))return state_=LocalReachabilityState::Degraded;dns_started_=true;if(!gateway_.start(network.ipv4,gateway_port_)){dns_.stop();dns_started_=false;return state_=LocalReachabilityState::Degraded;}gateway_started_=true;return state_=LocalReachabilityState::Ready;}void LocalReachability::stop()noexcept{if(state_==LocalReachabilityState::Unavailable&&!dns_started_&&!gateway_started_)return;if(gateway_started_){gateway_.stop();gateway_started_=false;}if(dns_started_){dns_.stop();dns_started_=false;}static_cast<void>(network_.stop());state_=LocalReachabilityState::Unavailable;} }
