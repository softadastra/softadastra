#include "host/LocalDns.hpp"
#include <gtest/gtest.h>
#include <array>
#include <string>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif
namespace {
#if defined(__linux__)
void put16(std::string &v,std::uint16_t n){v.push_back(n>>8);v.push_back(n);}
std::uint16_t get16(const std::string &v,std::size_t p){return (static_cast<unsigned char>(v[p])<<8)|static_cast<unsigned char>(v[p+1]);}
std::string query(std::uint16_t id,const std::string &name,std::uint16_t type=1){std::string q;put16(q,id);put16(q,0x0100);put16(q,1);q.append(6,'\0');std::size_t p=0;while(p<name.size()){auto e=name.find('.',p);if(e==std::string::npos)e=name.size();q.push_back(e-p);q.append(name,p,e-p);p=e+1;}q.push_back('\0');put16(q,type);put16(q,1);return q;}
std::string ask(std::uint16_t port,const std::string&q,const char *address="127.0.0.1"){int fd=::socket(AF_INET,SOCK_DGRAM,0);timeval t{0,200000};::setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&t,sizeof(t));sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons(port);::inet_pton(AF_INET,address,&a.sin_addr);::sendto(fd,q.data(),q.size(),0,reinterpret_cast<sockaddr*>(&a),sizeof(a));std::array<char,512>b{};auto n=::recv(fd,b.data(),b.size(),0);::close(fd);return n>0?std::string(b.data(),n):std::string{};}
TEST(LocalDnsTest, AnswersAnyEligibleLocalLabelAndPreservesTransaction){softadastra::LocalDns dns;ASSERT_TRUE(dns.start("127.0.0.1",0));const auto r=ask(dns.status().port,query(0x1234,"phone-test.softadastra.home.arpa"));ASSERT_GE(r.size(),28U);EXPECT_EQ(get16(r,0),0x1234);EXPECT_EQ(get16(r,6),1);EXPECT_EQ(static_cast<unsigned char>(r[r.size()-4]),127);EXPECT_EQ(static_cast<unsigned char>(r[r.size()-3]),0);dns.stop();}
TEST(LocalDnsTest, RejectsExternalInvalidAndExtraLabels){softadastra::LocalDns dns;ASSERT_TRUE(dns.start("127.0.0.1",0));for(const auto &n:{"example.com","foo.bar.softadastra.home.arpa","BAD.softadastra.home.arpa"}){const auto r=ask(dns.status().port,query(1,n));ASSERT_GE(r.size(),12U);EXPECT_EQ(get16(r,2)&15,3);}dns.stop();}
TEST(LocalDnsTest, ReturnsNoAnswerForUnsupportedTypeAndFormerrForMalformed){softadastra::LocalDns dns;ASSERT_TRUE(dns.start("127.0.0.1",0));const auto aaaa=ask(dns.status().port,query(2,"api.softadastra.home.arpa",28));ASSERT_GE(aaaa.size(),12U);EXPECT_EQ(get16(aaaa,6),0);const auto bad=ask(dns.status().port,"\x12\x34");EXPECT_TRUE(bad.empty());dns.stop();}
TEST(LocalDnsTest, UsesAddressFromEachStartAndStops){softadastra::LocalDns dns;ASSERT_TRUE(dns.start("127.0.0.1",0));auto port=dns.status().port;EXPECT_EQ(static_cast<unsigned char>(ask(port,query(3,"api.softadastra.home.arpa")).back()),1);dns.stop();EXPECT_TRUE(ask(port,query(3,"api.softadastra.home.arpa")).empty());ASSERT_TRUE(dns.start("127.0.0.2",0));EXPECT_EQ(static_cast<unsigned char>(ask(dns.status().port,query(3,"api.softadastra.home.arpa"),"127.0.0.2").back()),2);dns.stop();}
#endif
}
