#include "host/LocalGateway.hpp"
#include "host/HttpProxy.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <sstream>
#include <thread>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace { constexpr std::size_t max_headers=32768,max_body=1048576;
bool send_all(int fd,std::string_view v){for(std::size_t n=0;n<v.size();){auto r=::send(fd,v.data()+n,v.size()-n,0);if(r<=0)return false;n+=static_cast<std::size_t>(r);}return true;}
void reply(int fd,int c,const char *r){static_cast<void>(send_all(fd,"HTTP/1.1 "+std::to_string(c)+" "+r+"\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"));}
std::string lower(std::string x){for(char &c:x)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c)));return x;}
bool read_request(int fd,softadastra::HttpProxyRequest &out,std::string &host){std::string raw;std::array<char,4096>b{};while(raw.find("\r\n\r\n")==std::string::npos){auto n=::recv(fd,b.data(),b.size(),0);if(n<=0||raw.size()+static_cast<std::size_t>(n)>max_headers)return false;raw.append(b.data(),static_cast<std::size_t>(n));}auto end=raw.find("\r\n\r\n"),first=raw.find("\r\n");if(first==std::string::npos)return false;std::istringstream line(raw.substr(0,first));std::string version,extra;if(!(line>>out.method>>out.target>>version)||line>>extra||version!="HTTP/1.1"||out.target.empty()||out.target.front()!='/')return false;std::optional<std::size_t> length;for(std::size_t p=first+2;p<end;){auto e=raw.find("\r\n",p),colon=raw.find(':',p);if(e==std::string::npos||colon==std::string::npos||colon>=e)return false;auto key=lower(raw.substr(p,colon-p));auto value=raw.substr(colon+1,e-colon-1);while(!value.empty()&&(value.front()==' '||value.front()=='\t'))value.erase(0,1);if(key=="transfer-encoding")return false;if(key=="content-length"){if(length||value.empty()||!std::all_of(value.begin(),value.end(),[](unsigned char c){return std::isdigit(c);}))return false;try{length=std::stoull(value);}catch(...){return false;}if(*length>max_body)return false;}if(key=="host"){if(!host.empty()&&host!=value)return false;host=value;}out.headers.emplace_back(std::move(key),std::move(value));p=e+2;}const auto need=length.value_or(0);while(raw.size()<end+4+need){auto n=::recv(fd,b.data(),b.size(),0);if(n<=0||raw.size()+static_cast<std::size_t>(n)>max_headers+max_body)return false;raw.append(b.data(),static_cast<std::size_t>(n));}out.body.assign(raw.data()+end+4,need);if(host.empty()||host.find('/')!=std::string::npos||host.find('@')!=std::string::npos)return false;auto colon=host.rfind(':');if(colon!=std::string::npos){if(host.find(':')!=colon||colon==0)return false;host.resize(colon);}host=lower(host);return !host.empty();}
}
namespace softadastra { class LocalGateway::Thread:public std::thread{public:using std::thread::thread;}; LocalGateway::LocalGateway(LocalGatewayTargetResolver&r)noexcept:resolver_(r){} LocalGateway::~LocalGateway(){stop();}
bool LocalGateway::start(std::string address,std::uint16_t port){if(listener_>=0)return false;int fd=::socket(AF_INET,SOCK_STREAM,0),yes=1;if(fd<0)return false;::setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);if(::inet_pton(AF_INET,address.c_str(),&a.sin_addr)!=1||::bind(fd,reinterpret_cast<sockaddr*>(&a),sizeof(a))||::listen(fd,16)){::close(fd);status_.state=LocalGatewayState::Failed;return false;}if(!start_from_socket(fd)){::close(fd);status_.state=LocalGatewayState::Failed;return false;}return true;}
bool LocalGateway::start_from_socket(int fd){if(listener_>=0||fd<0)return false;int type{},accepting{};socklen_t size=sizeof(type);if(::getsockopt(fd,SOL_SOCKET,SO_TYPE,&type,&size)||type!=SOCK_STREAM)return false;size=sizeof(accepting);if(::getsockopt(fd,SOL_SOCKET,SO_ACCEPTCONN,&accepting,&size)||!accepting)return false;sockaddr_in address{};size=sizeof(address);if(::getsockname(fd,reinterpret_cast<sockaddr*>(&address),&size)||address.sin_family!=AF_INET)return false;char text[INET_ADDRSTRLEN]{};if(!::inet_ntop(AF_INET,&address.sin_addr,text,sizeof(text)))return false;listener_=fd;stopping_=false;status_={LocalGatewayState::Running,text,ntohs(address.sin_port)};thread_=new Thread([this]{run();});return true;}
void LocalGateway::stop()noexcept{stopping_=true;if(listener_>=0){::shutdown(listener_,SHUT_RDWR);::close(listener_);listener_=-1;}if(thread_){thread_->join();delete thread_;thread_=nullptr;}status_={};} LocalGatewayStatus LocalGateway::status()const{return status_;}
void LocalGateway::run()noexcept{HttpProxy proxy(resolver_);while(!stopping_){int client=::accept(listener_,nullptr,nullptr);if(client<0)continue;HttpProxyRequest request;std::string host;if(!read_request(client,request,host))reply(client,400,"Bad Request");else static_cast<void>(send_all(client,proxy.forward(host,request)));::close(client);}}
}
