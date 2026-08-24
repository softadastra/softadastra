#include "platform/ManagedNetwork.hpp"
namespace softadastra { const char *managed_network_state_name(ManagedNetworkState state) noexcept { return state == ManagedNetworkState::Running ? "running" : "stopped"; } }
