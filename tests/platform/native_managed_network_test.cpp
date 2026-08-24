#include "platform/NativeManagedNetwork.hpp"

#include <gtest/gtest.h>

#include <map>
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iterator>

namespace
{
  class Network final : public softadastra::Network
  {
  public:
    std::string primary;
    [[nodiscard]] bool is_available() const noexcept override { return true; }
    [[nodiscard]] bool is_connected() const noexcept override { return true; }
    [[nodiscard]] softadastra::NetworkCapability network_capability() const override
    { return {softadastra::NetworkState::Available, "192.168.1.6", primary, softadastra::NetworkInterfaceType::Wifi, softadastra::LocalNetworkState::Existing, softadastra::ManagedNetworkCapability::Unavailable}; }
  };
  class Runner final : public softadastra::NmcliRunner
  {
  public:
    std::map<std::string, std::vector<softadastra::NmcliResult>> responses;
    std::vector<std::vector<std::string>> calls;
    softadastra::NmcliResult fallback{1,{}};
    [[nodiscard]] softadastra::NmcliResult run(const std::vector<std::string> &arguments) override
    { calls.push_back(arguments); std::string key; for(const auto &value:arguments) key += value + "|"; auto found=responses.find(key); if(found==responses.end()||found->second.empty()) return fallback; const auto result=found->second.front(); found->second.erase(found->second.begin()); return result; }
    void reply(std::vector<std::string> arguments, int code, std::string output) { std::string key; for(const auto &value:arguments) key += value + "|"; responses[key].push_back({code,std::move(output)}); }
  };
  void stopped(Runner &runner) { runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,"Wired:eth0\n"); }
  TEST(NativeManagedNetworkTest, UnavailableWhenNetworkManagerCannotRespond)
  { Network network; network.primary="eth0"; Runner runner; runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},1,{}); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.status().capability,softadastra::ManagedNetworkCapability::Unavailable); }
  TEST(NativeManagedNetworkTest, SelectsDisconnectedApWifiBesideEthernet)
  { Network network; network.primary="eth0"; Runner runner; stopped(runner); runner.reply({"-t","-f","DEVICE,TYPE,STATE","device","status"},0,"eth0:ethernet:connected\nwlan1:wifi:disconnected\n"); runner.reply({"-g","WIFI-PROPERTIES.AP","device","show","wlan1"},0,"yes\n"); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.status().capability,softadastra::ManagedNetworkCapability::Available); }
  TEST(NativeManagedNetworkTest, RejectsPrimaryConnectedAndWifiP2p)
  { Network network; network.primary="wlp108s0"; Runner runner; stopped(runner); runner.reply({"-t","-f","DEVICE,TYPE,STATE","device","status"},0,"wlp108s0:wifi:connected\np2p-dev-wlp108s0:wifi-p2p:disconnected\n"); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.status().capability,softadastra::ManagedNetworkCapability::Unavailable); }
  TEST(NativeManagedNetworkTest, SelectsSecondWifiWhenPrimaryIsWifi)
  { Network network; network.primary="wlan0"; Runner runner; stopped(runner); runner.reply({"-t","-f","DEVICE,TYPE,STATE","device","status"},0,"wlan0:wifi:connected\nwlan1:wifi:disconnected\n"); runner.reply({"-g","WIFI-PROPERTIES.AP","device","show","wlan1"},0,"yes\n"); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.status().capability,softadastra::ManagedNetworkCapability::Available); }
  TEST(NativeManagedNetworkTest, ReportsOnlyItsOwnActiveProfile)
  { Network network; network.primary="eth0"; Runner runner; runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,"softadastra-managed-network:wlan1\nother:wlan2\n"); runner.reply({"-g","IP4.ADDRESS","device","show","wlan1"},0,"10.42.0.1/24\n"); softadastra::NativeManagedNetwork managed(&runner,&network); const auto status=managed.status(); EXPECT_EQ(status.state,softadastra::ManagedNetworkState::Running); EXPECT_EQ(status.interface_name,"wlan1"); EXPECT_EQ(status.ipv4,"10.42.0.1"); }

  class StateHome
  { public: StateHome() { path=std::filesystem::temp_directory_path()/"softadastra-native-managed-network-test"; std::filesystem::remove_all(path); ::setenv("XDG_STATE_HOME",path.c_str(),1); } ~StateHome() { std::filesystem::remove_all(path); ::unsetenv("XDG_STATE_HOME"); } std::filesystem::path path; };
  void startable(Runner &runner, bool activate) { runner.fallback={0,{}}; runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,"Wired:eth0\n"); for(int i=0;i<2;++i) { runner.reply({"-t","-f","DEVICE,TYPE,STATE","device","status"},0,"eth0:ethernet:connected\nwlan1:wifi:disconnected\n"); runner.reply({"-g","WIFI-PROPERTIES.AP","device","show","wlan1"},0,"yes\n"); } for(int i=0;i<(activate?2:1);++i) runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,activate?"softadastra-managed-network:wlan1\n":"Wired:eth0\n"); if(activate) { runner.reply({"-g","IP4.ADDRESS","device","show","wlan1"},0,"10.42.0.1/24\n"); runner.reply({"-g","IP4.ADDRESS","device","show","wlan1"},0,"10.42.0.1/24\n"); } }
  TEST(NativeManagedNetworkTest, StartsOnlyAfterNetworkManagerReportsActiveIpv4)
  { StateHome state; Network network; network.primary="eth0"; Runner runner; startable(runner,true); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.start(),softadastra::ManagedNetworkStartResult::Started); const auto status=managed.status(); EXPECT_EQ(status.state,softadastra::ManagedNetworkState::Running); EXPECT_EQ(status.ipv4,"10.42.0.1"); }
  TEST(NativeManagedNetworkTest, FailedActivationNeverReportsRunning)
  { StateHome state; Network network; network.primary="eth0"; Runner runner; startable(runner,false); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_EQ(managed.start(),softadastra::ManagedNetworkStartResult::Failed); EXPECT_NE(managed.status().state,softadastra::ManagedNetworkState::Running); }
  TEST(NativeManagedNetworkTest, StopTargetsOnlySoftadastraProfileAndIsIdempotent)
  { Network network; network.primary="eth0"; Runner runner; runner.fallback={0,{}}; runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,"softadastra-managed-network:wlan1\nother:wlan2\n"); runner.reply({"-g","IP4.ADDRESS","device","show","wlan1"},0,"10.42.0.1/24\n"); softadastra::NativeManagedNetwork managed(&runner,&network); EXPECT_TRUE(managed.stop()); bool found=false; for(const auto &call:runner.calls) if(call==std::vector<std::string>{"connection","down","id","softadastra-managed-network"}) found=true; EXPECT_TRUE(found); runner.calls.clear(); runner.reply({"-t","-f","NAME,DEVICE","connection","show","--active"},0,"other:wlan2\n"); EXPECT_FALSE(managed.stop()); for(const auto &call:runner.calls) EXPECT_NE(call,std::vector<std::string>({"connection","down","id","softadastra-managed-network"})); }
  TEST(NativeManagedNetworkTest, PersistsPskPrivatelyWithoutPassingItToNmcli)
  {
    StateHome state; Network network; network.primary="eth0"; Runner runner; startable(runner,true);
    softadastra::NativeManagedNetwork managed(&runner,&network);
    ASSERT_EQ(managed.start(),softadastra::ManagedNetworkStartResult::Started);
    const auto directory=state.path/"softadastra"/"network";
    const auto config=directory/"managed-network"; const auto profile=directory/"managed-network.nmconnection";
    std::ifstream input(config); std::string ssid,psk; ASSERT_TRUE(static_cast<bool>(input>>ssid>>psk));
    std::ifstream keyfile(profile); std::string contents((std::istreambuf_iterator<char>(keyfile)),{});
    EXPECT_NE(contents.find("psk="+psk),std::string::npos);
    EXPECT_EQ((std::filesystem::status(directory).permissions()&std::filesystem::perms::all),std::filesystem::perms::owner_all);
    EXPECT_EQ((std::filesystem::status(profile).permissions()&std::filesystem::perms::all),std::filesystem::perms::owner_read|std::filesystem::perms::owner_write);
    for(const auto &call:runner.calls) for(const auto &argument:call) EXPECT_EQ(argument.find(psk),std::string::npos);
  }
}
