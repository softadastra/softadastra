#include "platform/NativeManagedNetwork.hpp"

#include "platform/NativeDataDirectory.hpp"
#include "platform/NativeNetwork.hpp"

#include <array>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#if defined(__linux__)
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#endif

namespace
{
  constexpr const char *profile_name = "softadastra-managed-network";
  class ProductionNmcliRunner final : public softadastra::NmcliRunner
  {
  public:
    [[nodiscard]] softadastra::NmcliResult run(const std::vector<std::string> &arguments) override;
  };

#if defined(__linux__)
  softadastra::NmcliResult ProductionNmcliRunner::run(const std::vector<std::string> &arguments)
  {
    int pipefd[2]; if (::pipe(pipefd) != 0) return {};
    const pid_t pid = ::fork(); if (pid < 0) { ::close(pipefd[0]); ::close(pipefd[1]); return {}; }
    if (pid == 0) {
      ::dup2(pipefd[1], STDOUT_FILENO); ::dup2(pipefd[1], STDERR_FILENO);
      ::close(pipefd[0]); ::close(pipefd[1]); ::setenv("LC_ALL", "C", 1);
      std::vector<char *> argv; argv.reserve(arguments.size()+2); argv.push_back(const_cast<char *>("nmcli")); for(const auto &argument:arguments) argv.push_back(const_cast<char *>(argument.c_str())); argv.push_back(nullptr);
      ::execvp("nmcli", argv.data()); _exit(127);
    }
    ::close(pipefd[1]); std::array<char, 512> buffer{}; std::string output; ssize_t size=0; while((size=::read(pipefd[0],buffer.data(),buffer.size()))>0) output.append(buffer.data(),static_cast<std::size_t>(size)); ::close(pipefd[0]); int status=0; ::waitpid(pid,&status,0); return {WIFEXITED(status)?WEXITSTATUS(status):-1,std::move(output)};
  }
  std::vector<std::string> lines(const std::string &value) { std::vector<std::string> values; std::istringstream input(value); std::string line; while(std::getline(input,line)) if(!line.empty()) values.push_back(line); return values; }
  softadastra::NmcliResult run_nmcli(softadastra::NmcliRunner *runner, const std::vector<std::string> &arguments)
  { static ProductionNmcliRunner production; return runner ? runner->run(arguments) : production.run(arguments); }
  std::optional<std::string> candidate_interface(softadastra::NmcliRunner *runner, const softadastra::Network *provided_network)
  {
    const softadastra::NativeNetwork native_network; const softadastra::Network &network=provided_network?*provided_network:static_cast<const softadastra::Network &>(native_network); const auto primary=network.network_capability().primary_interface;
    const auto devices=run_nmcli(runner,{"-t","-f","DEVICE,TYPE,STATE","device","status"}); if(devices.code!=0) return std::nullopt;
    for(const auto &line:lines(devices.output)) { const auto first=line.find(':'); const auto second=first==std::string::npos?first:line.find(':',first+1); if(first==std::string::npos||second==std::string::npos) continue; const auto device=line.substr(0,first); if(line.substr(first+1,second-first-1)!="wifi"||line.substr(second+1)!="disconnected"||device==primary) continue; const auto ap=run_nmcli(runner,{"-g","WIFI-PROPERTIES.AP","device","show",device}); if(ap.code==0 && (ap.output=="yes\n"||ap.output=="yes")) return device; }
    return std::nullopt;
  }
  struct Config { std::string ssid; std::string password; };
  std::optional<Config> configuration(bool create)
  {
    const auto directory=softadastra::NativeDataDirectory::path()/"network"; std::error_code error; if(create) { std::filesystem::create_directories(directory,error); if(error) return std::nullopt; ::chmod(directory.c_str(),0700); } else if(!std::filesystem::exists(directory,error)||error) return std::nullopt;
    const auto path=directory/"managed-network"; std::ifstream input(path); Config config; if(input>>config.ssid>>config.password) return config; if(!create) return std::nullopt;
    std::array<unsigned char,24> bytes{}; const int descriptor=::open("/dev/urandom",O_RDONLY); if(descriptor<0||::read(descriptor,bytes.data(),bytes.size())!=static_cast<ssize_t>(bytes.size())) { if(descriptor>=0)::close(descriptor); return std::nullopt; } ::close(descriptor);
    static constexpr char alphabet[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"; static constexpr char hex[]="0123456789abcdef"; config.ssid="Softadastra-"; for(std::size_t i=0;i<4;++i) { config.ssid+=hex[bytes[i]>>4]; config.ssid+=hex[bytes[i]&15]; } for(std::size_t i=4;i<20;++i) config.password+=alphabet[bytes[i]%62];
    const int file=::open(path.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600); if(file<0) return std::nullopt; const std::string content=config.ssid+"\n"+config.password+"\n"; const bool written=::write(file,content.data(),content.size())==static_cast<ssize_t>(content.size()); ::close(file); return written?std::optional<Config>(config):std::nullopt;
  }
  std::optional<std::filesystem::path> profile_file(
      const Config &config, const std::string &interface_name)
  {
    const auto path=softadastra::NativeDataDirectory::path()/"network"/"managed-network.nmconnection";
    const std::string content="[connection]\nid=softadastra-managed-network\ntype=wifi\ninterface-name="+interface_name+"\n\n[802-11-wireless]\nmode=ap\nssid="+config.ssid+"\n\n[802-11-wireless-security]\nkey-mgmt=wpa-psk\npsk="+config.password+"\n\n[ipv4]\nmethod=shared\n\n[ipv6]\nmethod=ignore\n";
    const int file=::open(path.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0600); if(file<0) return std::nullopt;
    const bool written=::write(file,content.data(),content.size())==static_cast<ssize_t>(content.size()); ::close(file); return written?std::optional<std::filesystem::path>(path):std::nullopt;
  }
#endif
}

namespace softadastra
{
  ManagedNetworkStatus NativeManagedNetwork::status() const
  {
#if defined(__linux__)
    const auto config=configuration(false);
    const auto active=run_nmcli(runner_,{"-t","-f","NAME,DEVICE","connection","show","--active"}); if(active.code!=0) return {};
    for(const auto &line:lines(active.output)) if(line.starts_with(std::string(profile_name)+":")) { const auto device=line.substr(std::string(profile_name).size()+1); const auto address=run_nmcli(runner_,{"-g","IP4.ADDRESS","device","show",device}); std::string ipv4; if(address.code==0&&!address.output.empty()) ipv4=address.output.substr(0,address.output.find('/')); return {ManagedNetworkCapability::Available,ManagedNetworkState::Running,device,ipv4,config?config->ssid:""}; }
    const auto candidate=candidate_interface(runner_,network_); if(!candidate) return {};
    return {ManagedNetworkCapability::Available,ManagedNetworkState::Stopped,{},{},config?config->ssid:""};
#else
    return {};
#endif
  }
  ManagedNetworkStartResult NativeManagedNetwork::start()
  {
#if defined(__linux__)
    const auto current=status(); if(current.state==ManagedNetworkState::Running) return ManagedNetworkStartResult::AlreadyRunning;
    const auto candidate=candidate_interface(runner_,network_); const auto config=configuration(true); if(!candidate||!config) return ManagedNetworkStartResult::Unavailable;
    const auto profile=profile_file(*config,*candidate); if(!profile||run_nmcli(runner_,{"connection","load",profile->string()}).code!=0) return ManagedNetworkStartResult::Failed;
    const auto result=run_nmcli(runner_,{"connection","up","id",profile_name,"ifname",*candidate}); if(result.code!=0) return ManagedNetworkStartResult::Failed;
    return status().state==ManagedNetworkState::Running ? ManagedNetworkStartResult::Started : ManagedNetworkStartResult::Failed;
#else
    return ManagedNetworkStartResult::Unavailable;
#endif
  }
  bool NativeManagedNetwork::stop()
  {
#if defined(__linux__)
    const auto current=status(); return current.state==ManagedNetworkState::Running && run_nmcli(runner_,{"connection","down","id",profile_name}).code==0;
#else
    return false;
#endif
  }
}
