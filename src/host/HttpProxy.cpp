#include "host/HttpProxy.hpp"
#include "host/NativeSocket.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <limits>
#include <set>
#include <sstream>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace {
constexpr std::size_t max_response = 1048576;
std::string lower(std::string x) { for (char &c:x) c=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return x; }
std::string title(std::string x) { bool upper=true; for(char &c:x){c=static_cast<char>(upper?std::toupper(static_cast<unsigned char>(c)):std::tolower(static_cast<unsigned char>(c)));upper=c=='-';} return x; }
int close_socket(softadastra::NativeSocket fd) {
#if defined(_WIN32)
  return closesocket(fd);
#else
  return ::close(fd);
#endif
}
bool send_all(softadastra::NativeSocket fd, std::string_view value)
{
  for (std::size_t sent = 0; sent < value.size();) {
    const std::size_t remaining = value.size() - sent;
#if defined(_WIN32)
    const int length = static_cast<int>(std::min(remaining, static_cast<std::size_t>(std::numeric_limits<int>::max())));
#else
    const std::size_t length = remaining;
#endif
    const auto result = ::send(fd, value.data() + sent, length, 0);
    if (result <= 0) return false;
    sent += static_cast<std::size_t>(result);
  }
  return true;
}
std::set<std::string> blocked(const std::vector<std::pair<std::string,std::string>>& headers) { std::set<std::string> out{"connection","proxy-connection","keep-alive","te","trailer","transfer-encoding","upgrade"}; for(const auto &[k,v]:headers)if(lower(k)=="connection"){std::istringstream s(v);std::string token;while(std::getline(s,token,',')){while(!token.empty()&&token.front()==' ')token.erase(0,1);out.insert(lower(token));}} return out; }
std::string error(int status,const char *reason) { return "HTTP/1.1 "+std::to_string(status)+" "+reason+"\r\nContent-Length: 0\r\nConnection: close\r\n\r\n"; }
std::string sanitize_response(const std::string &response) {
  const auto end=response.find("\r\n\r\n");
  const auto first=response.find("\r\n");
  if(end==std::string::npos||first==std::string::npos) return {};
  std::vector<std::pair<std::string,std::string>> headers;
  for(std::size_t at=first+2;at<end;){const auto next=response.find("\r\n",at),colon=response.find(':',at);if(next==std::string::npos||colon==std::string::npos||colon>=next)return {};headers.emplace_back(response.substr(at,colon-at),response.substr(colon+1,next-colon-1));at=next+2;}
  const auto hops=blocked(headers); std::string result=response.substr(0,first)+"\r\n";
  for(const auto &[key,value]:headers) if(!hops.contains(lower(key))) result+=title(lower(key))+":"+value+"\r\n";
  return result+"Connection: close\r\n\r\n"+response.substr(end+4);
}
}
namespace softadastra {
std::string HttpProxy::forward(std::string_view software,const HttpProxyRequest &request) const {
  if (software.empty() || request.method.empty() || request.target.empty() || request.target.front() != '/' || request.method.find_first_of(" \r\n") != std::string::npos || request.target.find_first_of("\r\n") != std::string::npos) return error(400,"Bad Request");
  const auto target=resolver_.resolve(software);
  if(target.result==LocalGatewayLookup::NotFound) return error(404,"Not Found");
  if(target.result==LocalGatewayLookup::Unavailable) return error(503,"Service Unavailable");
  NativeSocket fd=::socket(AF_INET,SOCK_STREAM,0); sockaddr_in address{}; address.sin_family=AF_INET; address.sin_port=htons(target.port); address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
  if(fd==InvalidSocket || ::connect(fd,reinterpret_cast<sockaddr*>(&address),sizeof(address))!=0){if(fd!=InvalidSocket)close_socket(fd);return error(502,"Bad Gateway");}
  std::string wire=request.method+" "+request.target+" HTTP/1.1\r\n"; const auto hops=blocked(request.headers);
  bool has_length=false; for(const auto &[name,value]:request.headers){const auto key=lower(name);if(key.empty()||key.find_first_of(" \t\r\n:")!=std::string::npos||value.find_first_of("\r\n")!=std::string::npos||key=="transfer-encoding"){close_socket(fd);return error(400,"Bad Request");}if(key=="content-length"){if(has_length||value.empty()||!std::all_of(value.begin(),value.end(),[](unsigned char c){return std::isdigit(c);})) {close_socket(fd);return error(400,"Bad Request");}try{if(std::stoull(value)!=request.body.size()){close_socket(fd);return error(400,"Bad Request");}}catch(...){close_socket(fd);return error(400,"Bad Request");}has_length=true;}if(!hops.contains(key))wire+=title(key)+": "+value+"\r\n";}
  if(!request.body.empty()&&!has_length) wire+="Content-Length: "+std::to_string(request.body.size())+"\r\n";
  wire+="Connection: close\r\n\r\n"+request.body;
  if(!send_all(fd,wire)){close_socket(fd);return error(502,"Bad Gateway");}
#if defined(_WIN32)
  ::shutdown(fd,SD_SEND);
#else
  ::shutdown(fd,SHUT_WR);
#endif
  std::string response;
  std::array<char, 8192> buffer{};
  for (;;) {
#if defined(_WIN32)
    const int length = static_cast<int>(buffer.size());
#else
    const std::size_t length = buffer.size();
#endif
    const auto received = ::recv(fd, buffer.data(), length, 0);
    if (received == 0) break;
    if (received < 0) {
      close_socket(fd);
      return error(502, "Bad Gateway");
    }
    const auto received_size = static_cast<std::size_t>(received);
    if (received_size > max_response - response.size()) {
      close_socket(fd);
      return error(502, "Bad Gateway");
    }
    response.append(buffer.data(), received_size);
  }
  close_socket(fd);
  if(response.empty()||!response.starts_with("HTTP/"))return error(502,"Bad Gateway");
  response=sanitize_response(response);
  return response.empty()?error(502,"Bad Gateway"):response;
}
}
