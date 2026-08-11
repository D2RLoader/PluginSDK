#include <D2RLPlugin/api.h>
#include <cstdint>

static constexpr D2RL::PluginInfo NetworkPingPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "network-ping-sample",
	.name        = "Network Ping Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Sends a plugin message and resolves its host-side player.",
	.flags       = D2RL::PluginFlags::Shared,
};

static constexpr uint16_t PingMessage = 1;
static constexpr uint16_t PongMessage = 2;

static const D2RL::NetworkServiceV1* network;
static const D2RL::ThreadServiceV1*  threads;
static D2RL::Network::ChannelHandle  channel = D2RL::Network::InvalidChannelHandle;

static void __cdecl ResolvePeerAndReply(const D2RL::PluginContext* context, void* userData) noexcept {
	const auto         peer   = static_cast<D2RL::Network::PeerHandle>(reinterpret_cast<uintptr_t>(userData));
	D2RL::PlayerHandle player = D2RL::InvalidPlayerHandle;
	if (network->getPeerPlayer(context, channel, peer, &player) != D2RL::Network::Result::Success || player == D2RL::InvalidPlayerHandle) {
		context->LogWarn("The network sample could not resolve the sending peer's player.");
		return;
	}
	context->LogInfo("The host resolved the ping to its authoritative player handle.");
	(void)network->sendToClient(context, channel, peer, PongMessage, nullptr, 0);
}

static void __cdecl OnHostMessage(const D2RL::PluginContext* context, D2RL::Network::ChannelHandle, D2RL::Network::PeerHandle peer, uint16_t messageId, const void*, uint32_t, void*) noexcept {
	if (context == nullptr || messageId != PingMessage) {
		return;
	}
	void* peerToken = reinterpret_cast<void*>(static_cast<uintptr_t>(peer));
	if (threads->runOnGameThread(context, ResolvePeerAndReply, peerToken) != D2RL::Threads::Result::Success) {
		context->LogWarn("The network sample could not queue host-side player resolution.");
	}
}

static void __cdecl OnClientMessage(const D2RL::PluginContext* context, D2RL::Network::ChannelHandle, uint16_t messageId, const void*, uint32_t, void*) noexcept {
	if (context != nullptr && messageId == PongMessage) {
		context->LogInfo("Network sample pong received.");
	}
}

static void __cdecl OnConnectionState(const D2RL::PluginContext* context, const D2RL::Network::ConnectionEvent* event, void*) noexcept {
	if (context != nullptr && D2RL::Network::HasConnectionEventField(event, D2RL::Network::ConnectionEventRequiredSize) && event->state == D2RL::Network::ConnectionState::Connected) {
		(void)network->sendToHost(context, channel, PingMessage, nullptr, 0);
	}
}

static auto NetworkPingCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	D2RL::Network::ChannelInfo info {
		.structSize = D2RL::Network::ChannelInfoSize,
	};
	if (network->getChannelInfo(command->plugin, channel, &info) == D2RL::Network::Result::Success && info.connectionState == D2RL::Network::ConnectionState::Connected) {
		return network->sendToHost(command->plugin, channel, PingMessage, nullptr, 0) == D2RL::Network::Result::Success ? D2RL::ConsoleCommandResult::Handled : D2RL::ConsoleCommandResult::Failed;
	}
	return network->connectToHost(command->plugin, channel) == D2RL::Network::Result::Success ? D2RL::ConsoleCommandResult::Handled : D2RL::ConsoleCommandResult::Failed;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &NetworkPingPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Network, D2RL::NetworkServiceV1Version, &network) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasNetworkServiceV1Field(network, D2RL::NetworkServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Thread, D2RL::ThreadServiceV1Version, &threads) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasThreadServiceV1Field(threads, D2RL::ThreadServiceV1RequiredSize)) {
		return false;
	}
	const D2RL::Network::ChannelRegistration registration {
		.structSize         = D2RL::Network::ChannelRegistrationSize,
		.localChannelId     = 1,
		.compatibilityToken = 0x4E455450494E4701ULL,
		.hostMessage        = OnHostMessage,
		.clientMessage      = OnClientMessage,
		.connectionState    = OnConnectionState,
	};
	if (network->registerChannel(context, &registration, &channel) != D2RL::Network::Result::Success) {
		return false;
	}
	return context->RegisterConsoleCommand("network-sample-ping", NetworkPingCommand, "Connect the sample channel and send a ping.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
