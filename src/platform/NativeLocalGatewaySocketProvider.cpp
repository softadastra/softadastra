#include "platform/NativeLocalGatewaySocketProvider.hpp"
#include <cstdlib>
#include <cstring>
#if defined(__linux__)
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace softadastra { NativeLocalGatewaySocketProvider::NativeLocalGatewaySocketProvider(Environment environment)noexcept:environment_(std::move(environment)){if(!environment_)environment_=std::getenv;} LocalGatewaySocket NativeLocalGatewaySocketProvider::acquire()const noexcept{
#if defined(__linux__)
const auto pid=environment_("LISTEN_PID"),fds=environment_("LISTEN_FDS");if(!pid||!fds)return {};char *end{};const auto expected=std::strtol(pid,&end,10);if(*pid=='\0'||*end!='\0'||expected!=::getpid())return {};end=nullptr;const auto count=std::strtol(fds,&end,10);if(*fds=='\0'||*end!='\0'||count!=1)return {LocalGatewaySocketState::Invalid,-1};constexpr int fd=3;int type{},accepting{};socklen_t size=sizeof(type);if(::getsockopt(fd,SOL_SOCKET,SO_TYPE,&type,&size)||type!=SOCK_STREAM)return {LocalGatewaySocketState::Invalid,-1};size=sizeof(accepting);if(::getsockopt(fd,SOL_SOCKET,SO_ACCEPTCONN,&accepting,&size)||!accepting)return {LocalGatewaySocketState::Invalid,-1};return {LocalGatewaySocketState::Available,fd};
#else
return {};
#endif
} }
