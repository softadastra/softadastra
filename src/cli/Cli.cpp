#include "cli/Cli.hpp"

#include "cli/AccessUrl.hpp"
#include "platform/QrCode.hpp"
#include "platform/ProcessSpec.hpp"
#include "platform/NativeDataDirectory.hpp"
#include "software/AccessPoint.hpp"
#include "software/ProjectConfig.hpp"
#include "software/ProjectIdentity.hpp"
#include "software/SoftwareId.hpp"
#include "software/SoftwareState.hpp"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace
{
using namespace softadastra;
const char *state_name(SoftwareState s) noexcept { switch(s) { case SoftwareState::Stopped:return "stopped"; case SoftwareState::Starting:return "starting"; case SoftwareState::Running:return "running"; case SoftwareState::Failed:return "failed"; } return "unknown"; }
std::string access_name(const std::optional<AccessPoint> &a) { return a ? std::string(AccessPoint::name(a->protocol())) + ":" + std::to_string(a->port()) : "-"; }
std::string command_name(const SoftwareEntry &e) { if(!e.declared_command().empty()) return e.declared_command(); const auto &s=e.process_spec(); if(s.executable()=="/bin/sh" && s.arguments().size()==2 && s.arguments()[0]=="-lc") return s.arguments()[1]; std::string v=s.executable(); for(const auto &a:s.arguments()) v+=" "+a; return v; }
void operation_error(const SoftwareOperationResult &r) { switch(r.error().value_or(SoftwareOperationError::LaunchFailed)) { case SoftwareOperationError::SoftwareUnknown:std::cerr<<"software is unknown";break; case SoftwareOperationError::AlreadyRunning:std::cerr<<"software is already running";break; case SoftwareOperationError::NotRunning:std::cerr<<"software is not running";break; case SoftwareOperationError::ExecutableNotFound:std::cerr<<"command could not be started";break; case SoftwareOperationError::PermissionDenied:std::cerr<<"permission denied";break; case SoftwareOperationError::LaunchFailed:std::cerr<<"launch failed";break; case SoftwareOperationError::ProcessExitedSuccessfully:std::cerr<<"process exited successfully";break; case SoftwareOperationError::ProcessExitedWithNonZeroCode:std::cerr<<"process exited with code "<<r.exit_code().value_or(-1);break; case SoftwareOperationError::StopFailed:std::cerr<<"stop failed";break; } }
void unknown_software(const std::string &n) { std::cerr<<"Software not found: "<<n<<"\n\nView registered software with:\n\n  softadastra list\n"; }
void no_project(const std::string &c) { if(c=="logs"||c=="remove") { std::cerr<<"No Softadastra project found.\n\nSelect a registered software explicitly:\n\n  softadastra "<<c<<" <name>\n"; return; } std::cerr<<"No Softadastra project found.\n\nInitialize the current project with:\n\n  softadastra init\n\nOr select a registered software explicitly:\n\n  softadastra "<<c<<" <name>\n"; }
void usage() { std::cout<<"Softadastra runs software on this Host.\n\nUsage:\n  softadastra <command> [arguments]\n\nProject:\n  init [name] [--command <command>] [--access http:port]\n  run [name]\n\nSoftware:\n  start [name]\n  stop [name]\n  restart [name]\n  status [name]\n  info [name]\n  access [name]\n  logs [name] [--follow]\n\nInventory:\n  list [--running|--stopped]\n\nHost:\n  connectivity\n  network info\n  remote enable <ipv4-address> <port>\n  remote disable\n\nAdvanced:\n  register <name> [--access http:port] -- <command> [arguments...]\n"; }
void command_usage(const std::string &c) { if(c=="logs") std::cout<<"Usage:\n  softadastra logs [software-name] [--follow]\n\nWithout a name:\n  Show logs for the current Softadastra project.\n\nWith a name:\n  Show logs for a registered software.\n\nOptions:\n  -f, --follow    Follow new log output\n"; else if(c=="access") std::cout<<"Usage:\n  softadastra access [software-name]\n\nWithout a name:\n  Show local access for the current project.\n\nWith a name:\n  Show local access for a registered software.\n\nExamples:\n  softadastra access\n  softadastra access api\n"; else if(c=="network") std::cout<<"Usage:\n  softadastra network info\n  softadastra network status\n  softadastra network start\n  softadastra network stop\n\nCommands:\n  info      Show current Host network state and capabilities\n  status    Show managed local network state\n  start     Start the managed local network\n  stop      Stop the managed local network\n"; else if(c=="init") std::cout<<"Usage: softadastra init [name] [--command <command>] [--access http:port]\n"; else if(c=="run") std::cout<<"Usage: softadastra run [name]\n"; else if(c=="list") std::cout<<"Usage: softadastra list [--running|--stopped]\n"; else if(c=="register") std::cout<<"Usage: softadastra register <name> [--access http:port] -- <command> [arguments...]\n"; else if(c=="connectivity") std::cout<<"Usage: softadastra connectivity\n"; else if(c=="remote") std::cout<<"Usage: softadastra remote enable <ipv4-address> <port>\n       softadastra remote disable\n"; else if(c=="start"||c=="stop"||c=="restart"||c=="status"||c=="info") std::cout<<"Usage: softadastra "<<c<<" [name]\n"; else { std::cerr<<"Unknown command: "<<c<<'\n'; usage(); } }
bool is_help(const std::string &s) { return s=="-h"||s=="--help"; }
std::optional<AccessPoint> parse_access(const std::string &v) { const auto p=v.find(':'); if(p==std::string::npos) return std::nullopt; const auto protocol=AccessPoint::protocol(v.substr(0,p)); const auto port=AccessUrl::port(v.substr(p+1)); return protocol&&port ? AccessPoint::create(*protocol,*port):std::nullopt; }

struct Target { SoftwareId id{""}; std::string name; std::optional<SoftwareEntry> entry; std::optional<std::filesystem::path> root; std::optional<ProjectConfig> config; };
std::optional<Target> resolve_target(ControlClient &client,const std::optional<std::string> &name,const std::string &command) {
  if(name) { Target t{SoftwareId(*name),*name,client.software(SoftwareId(*name)),std::nullopt,std::nullopt}; if(!t.entry) { unknown_software(*name); return std::nullopt; } return t; }
  std::string error; const auto cfg=ProjectConfigFile::find(std::filesystem::current_path(),&error); if(!error.empty()) { std::cerr<<error<<'\n'; return std::nullopt; }
  if(cfg) { Target t{SoftwareId(cfg->second.id.value()),cfg->second.name,client.software(SoftwareId(cfg->second.id.value())),cfg->first,cfg->second}; if(!t.entry) { unknown_software(cfg->second.name); return std::nullopt; } return t; }
  const auto legacy=ProjectIdentity::find(std::filesystem::current_path()); if(!legacy) { no_project(command); return std::nullopt; } const auto entry=client.software_by_project_identity(legacy->second); if(!entry) { std::cerr<<"This Softadastra project is not registered on this Host.\n"; return std::nullopt; } return Target{entry->id(),entry->id().value(),entry,legacy->first,std::nullopt};
}
bool sync_project(ControlClient &client,Target &t) {
  if(!t.entry) return true;
  if(!t.config) { if(t.root&&t.entry->project_identity()&&!client.update_project_root(*t.entry->project_identity(),t.root->string())) { std::cerr<<"failed to update project location on this Host\n"; return false; } t.entry=client.software(t.id); return true; }
  if(!t.root) return true;
  const auto existing=t.entry->project_identity(); const auto legacy=ProjectIdentity::find(*t.root);
  if(existing&&existing->value()!=t.config->id.value()&&(!legacy||legacy->second!=*existing)) { std::cerr<<"Software identifier is linked to another project: "<<t.id.value()<<'\n'; return false; }
  static_cast<void>(client.synchronize_software(t.id,ProcessSpec("/bin/sh",{"-lc",t.config->command},t.root->string()),t.config->access));
  if(existing&&!client.update_project_root(*existing,t.root->string())) { std::cerr<<"failed to update project location on this Host\n"; return false; } t.entry=client.software(t.id); return true;
}
bool print_access(ControlClient &client,const Target &t) {
  const auto configured=t.config?t.config->access:t.entry->access_point(); if(!configured) { std::cerr<<"No access configured for: "<<t.name<<'\n'; if(t.root&&t.config) std::cerr<<"\nConfigure `access` in:\n\n  "<<(*t.root/"softadastra.toml").string()<<"\n\nExample:\n\n  access = \"http:8080\"\n"; else std::cerr<<"\nThis global Software has no AccessPoint configured.\n"; return false; }
  const auto access=client.local_access(t.id); if(!access) { std::cerr<<"Local access information is unavailable\n"; return false; }
  const auto entry=client.software(t.id); if(!entry) { unknown_software(t.name); return false; }
  std::cout<<"Software:      "<<t.name<<"\nState:         "<<state_name(entry->state())<<"\nAccess:        "<<access_name(configured)<<'\n';
  if(access->state==LocalAccessState::Available) { std::cout<<"Network:       "<<local_access_network_name(access->network)<<"\nLocal URL:     "<<access->url<<"\n\n"; if(!QrCode::print(access->url)) std::cout<<"QR generation is unavailable.\n"; std::cout<<"\nScan with your phone.\n"; return true; }
  std::cout<<"Local access:  unavailable\n";
  if(entry->state()==SoftwareState::Stopped) std::cout<<"\nStart it with:\n\n  softadastra run"<<(t.root?"":" "+t.name)<<"\n";
  else if(entry->state()==SoftwareState::Failed) std::cout<<"\nInspect logs with:\n\n  softadastra logs"<<(t.root?"":" "+t.name)<<"\n";
  else if(access->local_network_state==LocalNetworkState::Unavailable) std::cout<<"\nNo active local network is available on this Host.\n\nManaged network capability: "<<managed_network_capability_name(access->managed_network_capability)<<"\n";
  return false;
}
}

namespace softadastra {
Cli::Cli(ControlClient &client) noexcept : client_(client) {}
Cli::Cli(ControlClient &client, const Network &network) noexcept : client_(client), network_(&network) {}
Cli::Cli(ControlClient &client, const Network &network, ManagedNetwork &managed_network) noexcept : client_(client), network_(&network), managed_network_(&managed_network) {}
int Cli::run(int argc,const char *const argv[]) {
  if(argc<2) { usage(); return 2; } const std::string command(argv[1]);
  if(command=="help") { if(argc==2) usage(); else if(argc==3 && std::string(argv[2])=="remove") std::cout<<"Usage:\n  softadastra remove [software-name]\n\nThe project files and logs are not deleted.\n"; else if(argc==3 && std::string(argv[2])=="logs") std::cout<<"Usage:\n  softadastra logs [software-name] [--follow|--clear]\n\nOptions:\n  -f, --follow    Follow new log output\n  --clear         Clear stored logs\n"; else if(argc==3) command_usage(argv[2]); else { std::cerr<<"help accepts one command\n"; return 2; } return 0; }
  if(is_help(command)) { usage(); return 0; }
  if(argc>=3&&is_help(argv[2])) { if(argc!=3) { command_usage(command); return 2; } if(command=="remove") std::cout<<"Usage:\n  softadastra remove [software-name]\n\nThe project files and logs are not deleted.\n"; else if(command=="logs") std::cout<<"Usage:\n  softadastra logs [software-name] [--follow|--clear]\n\nOptions:\n  -f, --follow    Follow new log output\n  --clear         Clear stored logs\n"; else command_usage(command); return 0; }
  if(command=="network") {
    if(argc!=3) { command_usage(command); return 2; }
    const std::string action(argv[2]);
    const auto status=managed_network_?std::optional<ManagedNetworkStatus>(managed_network_->status()):client_.managed_network_status();
    if(action=="start") {
      const auto result=managed_network_?std::optional<ManagedNetworkStartResult>(managed_network_->start()):client_.start_managed_network();
      if(!result||*result==ManagedNetworkStartResult::Unavailable) { std::cerr<<"Managed network is unavailable on this Host.\n"; return 1; }
      if(*result==ManagedNetworkStartResult::WouldDisruptConnection) { std::cerr<<"Managed network cannot be started without disrupting the current network connection.\n"; return 1; }
      if(*result==ManagedNetworkStartResult::Failed) { std::cerr<<"Failed to start Softadastra local network.\n"; return 1; }
      const auto current=managed_network_?std::optional<ManagedNetworkStatus>(managed_network_->status()):client_.managed_network_status(); if(!current) return 1;
      std::cout<<(*result==ManagedNetworkStartResult::AlreadyRunning?"Softadastra local network is already running.\n":"Softadastra local network started.\n")<<"\nNetwork:   "<<current->ssid<<"\nIPv4:      "<<current->ipv4<<'\n'; return 0;
    }
    if(action=="stop") { const auto stopped=managed_network_?std::optional<bool>(managed_network_->status().state==ManagedNetworkState::Running&&managed_network_->stop()):client_.stop_managed_network(); if(!stopped||!*stopped) { std::cout<<"Softadastra local network is not running.\n"; return 0; } std::cout<<"Softadastra local network stopped.\n"; return 0; }
    if(action=="status") { if(!status) { std::cerr<<"Managed network status is unavailable.\n"; return 1; } std::cout<<"Managed network: "<<managed_network_state_name(status->state)<<'\n'; return 0; }
    if(action!="info") { command_usage(command); return 2; }
    const auto capability=network_?std::optional<NetworkCapability>(network_->network_capability()):client_.network_capability(); if(!capability) { std::cerr<<"Host network information is unavailable\n"; return 1; }
    const auto managed=status.value_or(ManagedNetworkStatus{}); std::cout<<"State:            "<<network_state_name(capability->state)<<"\nPrimary IPv4:     "<<(capability->primary_ipv4.empty()?"-":capability->primary_ipv4)<<"\nInterface:        "<<(capability->primary_interface.empty()?"-":capability->primary_interface)<<"\nType:             "<<network_interface_type_name(capability->interface_type)<<"\nLocal network:    "<<local_network_state_name(capability->local_network_state)<<"\nManaged network:  "<<managed_network_capability_name(managed.capability); if(managed.capability==ManagedNetworkCapability::Available) { std::cout<<"\nManaged state:    "<<managed_network_state_name(managed.state); if(managed.state==ManagedNetworkState::Running) std::cout<<"\nManaged interface: "<<managed.interface_name<<"\nManaged IPv4:      "<<managed.ipv4<<"\nManaged SSID:      "<<managed.ssid; } std::cout<<'\n'; return 0;
  }
  if(command=="init") {
    std::error_code ec; const auto root=std::filesystem::weakly_canonical(std::filesystem::current_path(),ec); if(ec) { std::cerr<<"failed to determine current working directory\n"; return 1; } const auto path=root/"softadastra.toml"; if(std::filesystem::exists(path,ec)) { std::cout<<"Softadastra project already initialized:\n\n  "<<path.string()<<'\n'; return 0; }
    std::string name=root.filename().string(), configured_command; std::optional<AccessPoint> access; for(int i=2;i<argc;++i) { const std::string v(argv[i]); if(v=="--command"&&i+1<argc) configured_command=argv[++i]; else if(v=="--access"&&i+1<argc) { access=parse_access(argv[++i]); if(!access) { std::cerr<<"access point must use http:port or https:port with port 1 to 65535\n"; return 2; } } else if(!v.starts_with("-")&&name==root.filename().string()) name=v; else { std::cerr<<"invalid init arguments\n"; return 2; } }
    ProjectIdentity id=ProjectIdentity::generate(); const auto legacy=ProjectIdentity::find(root); if(legacy&&client_.host_available()) { const auto entry=client_.software_by_project_identity(legacy->second); if(entry) id=ProjectIdentity(entry->id().value()); }
    if(!ProjectConfigFile::create(root,ProjectConfig{id,name,configured_command,access})) { std::cerr<<"failed to create "<<path.string()<<'\n'; return 1; } std::cout<<"Created softadastra.toml\n"; if(configured_command.empty()) std::cout<<"\nSet `command` in softadastra.toml, then run:\n\n  softadastra run\n"; return 0;
  }
  if(!client_.host_available()) { std::cerr<<"Softadastra Host is unavailable\n"; return 1; }
  if(command=="list") { if(argc>3||(argc==3&&std::string(argv[2])!="--running"&&std::string(argv[2])!="--stopped")) { std::cerr<<"list accepts only --running or --stopped\n"; return 2; } const auto filter=argc==3?std::optional<SoftwareState>(std::string(argv[2])=="--running"?SoftwareState::Running:SoftwareState::Stopped):std::nullopt; std::cout<<std::left<<std::setw(12)<<"NAME"<<std::setw(11)<<"STATE"<<std::setw(13)<<"ACCESS"<<"PROJECT\n"; for(const auto &e:client_.software()) if(!filter||e.state()==*filter) std::cout<<std::left<<std::setw(12)<<e.id().value()<<std::setw(11)<<state_name(e.state())<<std::setw(13)<<access_name(e.access_point())<<e.process_spec().working_directory().value_or("-")<<'\n'; return 0; }
  if(command=="connectivity") { if(argc!=2) { command_usage(command); return 2; } std::cout<<"network: "<<(client_.connectivity_available()?"available":"unavailable")<<'\n'; if(client_.connectivity_available()) std::cout<<"connected: "<<(client_.connected()?"yes":"no")<<'\n'; return 0; }
  if(command=="remote") { if((argc!=3||std::string(argv[2])!="disable")&&(argc!=5||std::string(argv[2])!="enable")) { command_usage(command); return 2; } const auto r=client_.request(argc==3?"remote disable":"remote enable "+std::string(argv[3])+" "+argv[4]); if(!r||*r=="error") { std::cerr<<"failed to update remote access\n"; return 1; } std::cout<<(*r=="remote 1"?"remote access: enabled\n":"remote access: disabled\n"); return 0; }
  if(command=="register") { if(argc<4) { command_usage(command); return 2; } int executable=3; std::optional<AccessPoint> access; if(std::string(argv[3])=="--access") { if(argc<7||std::string(argv[5])!="--") { command_usage(command); return 2; } access=parse_access(argv[4]); if(!access) { std::cerr<<"access point must use http:port or https:port with port 1 to 65535\n"; return 2; } executable=6; } else if(std::string(argv[3])=="--") { if(argc<5) { command_usage(command); return 2; } executable=4; } const SoftwareId id(argv[2]); if(client_.software(id)) { std::cerr<<"Software already registered: "<<id.value()<<"\n\nChoose a different software name.\n"; return 1; } std::vector<std::string> args; for(int i=executable+1;i<argc;++i) args.emplace_back(argv[i]); std::error_code ec; const auto root=std::filesystem::weakly_canonical(std::filesystem::current_path(),ec); if(ec) return 1; if(!client_.register_software(id,ProcessSpec(argv[executable],std::move(args),root.string()),access)) { std::cerr<<"failed to register software: "<<id.value()<<'\n'; return 1; } std::cout<<"registered: "<<id.value()<<'\n'; return 0; }
  const bool valid=command=="run"||command=="start"||command=="stop"||command=="restart"||command=="status"||command=="info"||command=="access"||command=="logs"||command=="remove"; if(!valid) { std::cerr<<"Unknown command: "<<command<<'\n'; usage(); return 2; }
  bool follow=false, clear=false; std::optional<std::string> name;
  if(command=="logs") { for(int i=2;i<argc;++i) { const std::string value(argv[i]); if(value=="-f"||value=="--follow") follow=true; else if(value=="--clear") clear=true; else if(!name) name=value; else { command_usage(command); return 2; } } if(follow&&clear) { std::cerr<<"--clear cannot be used with --follow\n"; return 2; } }
  else { if(argc>3) { command_usage(command); return 2; } name=argc==3?std::optional<std::string>(argv[2]):std::nullopt; }
  if(command=="run"&&!name) { std::string error; const auto cfg=ProjectConfigFile::find(std::filesystem::current_path(),&error); if(!error.empty()) { std::cerr<<error<<'\n'; return 1; } if(cfg&&cfg->second.command.empty()) { std::cerr<<"No command configured for: "<<cfg->second.name<<"\n\nSet `command` in:\n\n  "<<(cfg->first/"softadastra.toml").string()<<'\n'; return 1; } if(cfg&&!client_.software(SoftwareId(cfg->second.id.value()))) { const auto legacy=ProjectIdentity::find(cfg->first); const auto identity=legacy?legacy->second:cfg->second.id; if(!client_.register_software(SoftwareId(cfg->second.id.value()),ProcessSpec("/bin/sh",{"-lc",cfg->second.command},cfg->first.string()),cfg->second.access,identity)) { std::cerr<<"Failed to start software: "<<cfg->second.name<<'\n'; return 1; } } }
  auto target=resolve_target(client_,name,command); if(!target) return 1; if(!sync_project(client_,*target)) return 1;
  if(command=="remove") { if(target->entry->state()==SoftwareState::Running) { std::cerr<<"Cannot remove running software: "<<target->name<<"\n\nStop it first:\n\n  softadastra stop"<<(name?" "+*name:"")<<"\n"; return 1; } if(!client_.remove_software(target->id)) { unknown_software(target->name); return 1; } std::cout<<"removed: "<<target->name<<'\n'; return 0; }
  if(command=="logs") { const auto path=NativeDataDirectory::path()/"logs"/(target->id.value()+".log"); if(clear) { std::ofstream output(path, std::ios::trunc); return 0; } std::uintmax_t offset=0; do { std::ifstream input(path, std::ios::binary); if(input) { input.seekg(static_cast<std::streamoff>(offset)); std::cout<<input.rdbuf()<<std::flush; const auto size=std::filesystem::file_size(path); offset=size; } if(!follow) break; std::this_thread::sleep_for(std::chrono::milliseconds(150)); } while(true); return 0; }
  if(command=="access") return print_access(client_,*target)?0:1;
  if(command=="info") { const auto &e=*target->entry; std::cout<<"Name:       "<<target->name<<"\nState:      "<<state_name(e.state())<<"\nCommand:    "<<(target->config?target->config->command:command_name(e))<<"\nProject:    "<<(target->root?target->root->string():e.process_spec().working_directory().value_or("-"))<<"\nAccess:     "<<access_name(target->config?target->config->access:e.access_point())<<"\nPID:        "<<(e.pid()?std::to_string(*e.pid()):"-")<<"\n"; return 0; }
  if(command=="status") { const auto s=client_.software_state(target->id); if(!s) { unknown_software(target->name); return 1; } std::cout<<target->name<<": "<<state_name(*s)<<'\n'; return 0; }
  if(command=="start"||command=="run") { const auto s=client_.software_state(target->id); const bool running=s&&*s==SoftwareState::Running; if(!running) { const auto result=client_.start_software(target->id); if(!result) { std::cerr<<"Failed to start software: "<<target->name<<"\n\nCommand:\n  "<<(target->config?target->config->command:command_name(*target->entry))<<"\n\nProject:\n  "<<(target->root?target->root->string():target->entry->process_spec().working_directory().value_or("-"))<<"\n\nReason:\n  "; operation_error(result); std::cerr<<'\n'; return 1; } } std::cout<<(running?"already running: ":(command=="run"?"running: ":"started: "))<<target->name<<'\n'; if(command=="run"&&(target->config?target->config->access.has_value():target->entry->access_point().has_value())) static_cast<void>(print_access(client_,*target)); return 0; }
  const auto result=command=="stop"?client_.stop_software(target->id):client_.restart_software(target->id); if(!result) { std::cerr<<"Failed to "<<command<<" software: "<<target->name<<"\n\nReason:\n  "; operation_error(result); std::cerr<<'\n'; return 1; } std::cout<<(command=="stop"?"stopped: ":"restarted: ")<<target->name<<'\n'; return 0;
}
}
