#include "host/RemoteReachability.hpp"
#include "host/HttpProxy.hpp"
#include <array>
#include <chrono>
#include <limits>
#include <mutex>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace { constexpr std::size_t max_frame=1024*1024,max_headers=128,max_field=32768;
int close_socket(int fd){
#if defined(_WIN32)
 return closesocket(static_cast<SOCKET>(fd));
#else
 return ::close(fd);
#endif
}
bool send_all(int fd, std::string_view value)
{
 for (std::size_t sent = 0; sent < value.size();) {
  const std::size_t remaining = value.size() - sent;
#if defined(_WIN32)
  if (remaining > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
  const int length = static_cast<int>(remaining);
#else
  const std::size_t length = remaining;
#endif
  const auto result = ::send(fd, value.data() + sent, length, 0);
  if (result <= 0) return false;
  sent += static_cast<std::size_t>(result);
 }
 return true;
}
bool receive_all(int fd, char *out, std::size_t size)
{
 for (std::size_t received = 0; received < size;) {
  const std::size_t remaining = size - received;
#if defined(_WIN32)
  if (remaining > static_cast<std::size_t>(std::numeric_limits<int>::max())) return false;
  const int length = static_cast<int>(remaining);
#else
  const std::size_t length = remaining;
#endif
  const auto result = ::recv(fd, out + received, length, 0);
  if (result <= 0) return false;
  received += static_cast<std::size_t>(result);
 }
 return true;
}
std::uint16_t u16(const char *p){return (static_cast<unsigned char>(p[0])<<8)|static_cast<unsigned char>(p[1]);}
std::uint32_t u32(const char *p){return (static_cast<std::uint32_t>(static_cast<unsigned char>(p[0]))<<24)|(static_cast<std::uint32_t>(static_cast<unsigned char>(p[1]))<<16)|(static_cast<std::uint32_t>(static_cast<unsigned char>(p[2]))<<8)|static_cast<unsigned char>(p[3]);}
void put32(std::string &v,std::uint32_t x){v.push_back(static_cast<char>(x>>24));v.push_back(static_cast<char>(x>>16));v.push_back(static_cast<char>(x>>8));v.push_back(static_cast<char>(x));}
bool read_frame(int fd,std::string &frame){std::array<char,4> h{};if(!receive_all(fd,h.data(),h.size()))return false;auto n=u32(h.data());if(n==0||n>max_frame)return false;frame.resize(n);return receive_all(fd,frame.data(),n);}
bool write_frame(int fd,const std::string &frame){if(frame.size()>max_frame)return false;std::string h;put32(h,static_cast<std::uint32_t>(frame.size()));return send_all(fd,h)&&send_all(fd,frame);}
bool take(const std::string &v,std::size_t &at,std::size_t n,std::string &out){if(n>max_field||at>v.size()||n>v.size()-at)return false;out.assign(v.data()+at,n);at+=n;return true;}
bool parse_request(const std::string &f,std::string &software,softadastra::HttpProxyRequest &q){if(f.empty()||f[0]!=1)return false;std::size_t at=1;if(at+2>f.size())return false;auto n=u16(f.data()+at);at+=2;if(!take(f,at,n,software)||at+2>f.size())return false;n=u16(f.data()+at);at+=2;if(!take(f,at,n,q.method)||at+4>f.size())return false;auto path=u32(f.data()+at);at+=4;if(!take(f,at,path,q.target)||at+2>f.size())return false;auto count=u16(f.data()+at);at+=2;if(count>max_headers)return false;for(std::size_t i=0;i<count;++i){if(at+2>f.size())return false;n=u16(f.data()+at);at+=2;std::string key,value;if(!take(f,at,n,key)||at+2>f.size())return false;n=u16(f.data()+at);at+=2;if(!take(f,at,n,value))return false;q.headers.emplace_back(std::move(key),std::move(value));}if(at+4>f.size())return false;auto body=u32(f.data()+at);at+=4;return take(f,at,body,q.body)&&at==f.size();}
int connect_to(const softadastra::RemoteEndpoint &endpoint,std::atomic_int &active){
#if defined(_WIN32)
 static std::once_flag winsock_once; static bool winsock_ready=false;
 std::call_once(winsock_once,[]{WSADATA data{};winsock_ready=WSAStartup(MAKEWORD(2,2),&data)==0;});
 if(!winsock_ready)return -1;
#endif
addrinfo hints{};hints.ai_family=AF_UNSPEC;hints.ai_socktype=SOCK_STREAM;addrinfo *found{};const auto port=std::to_string(endpoint.port);if(::getaddrinfo(endpoint.address.c_str(),port.c_str(),&hints,&found)!=0)return -1;int fd=-1;for(auto *it=found;it;it=it->ai_next){fd=::socket(it->ai_family,it->ai_socktype,it->ai_protocol);if(fd<0)continue;active=fd;if(::connect(fd,it->ai_addr,it->ai_addrlen)==0)break;if(active.exchange(-1)==fd)close_socket(fd);fd=-1;}::freeaddrinfo(found);return fd;}
}
namespace softadastra {
RemoteReachability::RemoteReachability(LocalGatewayTargetResolver &resolver) noexcept:resolver_(resolver){} RemoteReachability::~RemoteReachability(){disable();}
RemoteReachabilityState RemoteReachability::state()const noexcept{std::lock_guard lock(mutex_);return state_;} RemoteEndpoint RemoteReachability::endpoint()const{std::lock_guard lock(mutex_);return endpoint_;}
void RemoteReachability::configure(RemoteEndpoint endpoint){disable();if(!endpoint.valid())return;{std::lock_guard lock(mutex_);endpoint_=std::move(endpoint);state_=RemoteReachabilityState::Connecting;stopping_=false;}thread_=std::thread(&RemoteReachability::run,this);}
void RemoteReachability::disable()noexcept{stopping_=true;const auto fd=socket_.exchange(-1);if(fd>=0){
#if defined(_WIN32)
::shutdown(fd,SD_BOTH);
#else
::shutdown(fd,SHUT_RDWR);
#endif
close_socket(fd);}wake_.notify_all();if(thread_.joinable())thread_.join();std::lock_guard lock(mutex_);endpoint_={};state_=RemoteReachabilityState::Disabled;}
void RemoteReachability::run()noexcept{HttpProxy proxy(resolver_);auto delay=std::chrono::milliseconds(50);while(!stopping_){RemoteEndpoint endpoint;{std::lock_guard lock(mutex_);endpoint=endpoint_;state_=RemoteReachabilityState::Connecting;}int fd=connect_to(endpoint,socket_);if(fd<0||stopping_){if(fd>=0&&socket_.exchange(-1)==fd)close_socket(fd);break;}delay=std::chrono::milliseconds(50);{std::lock_guard lock(mutex_);if(!stopping_)state_=RemoteReachabilityState::Ready;}bool ok=true;while(!stopping_&&ok){std::string frame,software;HttpProxyRequest request;ok=read_frame(fd,frame)&&parse_request(frame,software,request);if(ok){std::string answer(1,2);answer+=proxy.forward(software,request);ok=write_frame(fd,answer);}}if(socket_.exchange(-1)==fd)close_socket(fd);if(!stopping_){std::unique_lock lock(mutex_);state_=RemoteReachabilityState::Degraded;wake_.wait_for(lock,delay,[this]{return stopping_.load();});delay=std::min(delay*2,std::chrono::milliseconds(1000));}}}
}
