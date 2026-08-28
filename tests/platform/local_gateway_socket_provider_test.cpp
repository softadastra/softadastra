#include "platform/NativeLocalGatewaySocketProvider.hpp"
#include <gtest/gtest.h>
#include <map>
#include <string>
#if defined(__linux__)
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif
namespace
{
#if defined(__linux__)
  softadastra::NativeLocalGatewaySocketProvider provider(std::map<std::string, std::string> values)
  {
    return softadastra::NativeLocalGatewaySocketProvider([values = std::move(values)](const char *key) -> const char *
                                                         {const auto it=values.find(key);return it==values.end()?nullptr:it->second.c_str(); });
  }
  std::map<std::string, std::string> activation(std::string fds = "1") { return {{"LISTEN_PID", std::to_string(::getpid())}, {"LISTEN_FDS", std::move(fds)}}; }
  int listening()
  {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in a{};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (fd < 0 || ::bind(fd, reinterpret_cast<sockaddr *>(&a), sizeof(a)) || ::listen(fd, 1))
    {
      if (fd >= 0)
        ::close(fd);
      return -1;
    }
    return fd;
  }
  TEST(LocalGatewaySocketProviderTest, IsUnavailableWithoutActivationVariables)
  {
    EXPECT_EQ(provider({}).acquire().state, softadastra::LocalGatewaySocketState::Unavailable);
    EXPECT_EQ(provider({{"LISTEN_PID", std::to_string(::getpid())}}).acquire().state, softadastra::LocalGatewaySocketState::Unavailable);
  }
  TEST(LocalGatewaySocketProviderTest, RejectsWrongPidAndMultipleDescriptors)
  {
    auto wrong = activation();
    wrong["LISTEN_PID"] = "1";
    EXPECT_EQ(provider(wrong).acquire().state, softadastra::LocalGatewaySocketState::Unavailable);
    EXPECT_EQ(provider(activation("2")).acquire().state, softadastra::LocalGatewaySocketState::Invalid);
  }
  TEST(LocalGatewaySocketProviderTest, AcquiresOnlyListeningTcpSocketAtStandardDescriptor)
  {
    const int fd = listening();
    ASSERT_GE(fd, 0);
    ASSERT_EQ(::dup2(fd, 3), 3);
    if (fd != 3)
      ::close(fd);
    const auto result = provider(activation()).acquire();
    EXPECT_EQ(result.state, softadastra::LocalGatewaySocketState::Available);
    EXPECT_EQ(result.fd, 3);
    ::close(3);
  }
  TEST(LocalGatewaySocketProviderTest, RejectsNonSocketAndNonListeningSocket)
  {
    int pipefd[2]{};
    ASSERT_EQ(::pipe(pipefd), 0);
    ASSERT_EQ(::dup2(pipefd[0], 3), 3);
    EXPECT_EQ(provider(activation()).acquire().state, softadastra::LocalGatewaySocketState::Invalid);
    ::close(pipefd[0]);
    ::close(pipefd[1]);
    ::close(3);
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_GE(fd, 0);
    ASSERT_EQ(::dup2(fd, 3), 3);
    if (fd != 3)
      ::close(fd);
    EXPECT_EQ(provider(activation()).acquire().state, softadastra::LocalGatewaySocketState::Invalid);
    ::close(3);
  }
#endif
}
