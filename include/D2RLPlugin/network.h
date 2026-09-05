#pragma once

#include <D2RLPlugin/handles.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Network {

using ChannelHandle = uint64_t;
using PeerHandle    = uint64_t;

inline constexpr ChannelHandle InvalidChannelHandle = 0;
inline constexpr PeerHandle    InvalidPeerHandle    = 0;
inline constexpr uint32_t      MaxPayloadSize       = 224;

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	Unavailable     = 3,
	Conflict        = 4,
	NotFound        = 5,
	Busy            = 6,
	OwnerInactive   = 7,
	OwnerMismatch   = 8,
	StaleHandle     = 9,
	CallbackFault   = 10,
	PayloadTooLarge = 11,
	NotConnected    = 12,
	WrongRole       = 13,
	Incompatible    = 14,
	RateLimited     = 15,
};

enum class ChannelState : uint32_t {
	Unknown = 0,
	Staged  = 1,
	Active  = 2,
	Closing = 3,
};

enum class ConnectionState : uint32_t {
	Disconnected = 0,
	Connecting   = 1,
	Connected    = 2,
	Rejected     = 3,
};

enum class RejectionReason : uint32_t {
	None               = 0,
	UnknownPlugin      = 1,
	UnknownChannel     = 2,
	ScopeMismatch      = 3,
	Incompatible       = 4,
	ProtocolMismatch   = 5,
	HostUnavailable    = 6,
	RegistrationClosed = 7,
};

struct ConnectionEvent {
	uint32_t        structSize;
	uint32_t        flags;
	ChannelHandle   channel;
	PeerHandle      peer;
	ConnectionState state;
	RejectionReason reason;
};

using HostMessageCallback = void(__cdecl*)(const PluginContext* context, ChannelHandle channel, PeerHandle peer, uint16_t messageId, const void* data, uint32_t size, void* userData) noexcept;

using ClientMessageCallback = void(__cdecl*)(const PluginContext* context, ChannelHandle channel, uint16_t messageId, const void* data, uint32_t size, void* userData) noexcept;

using ConnectionStateCallback = void(__cdecl*)(const PluginContext* context, const ConnectionEvent* event, void* userData) noexcept;

// localChannelId is private to this plugin. D2RLoader also adds the PluginContext
// id and load scope, so two plugins may use the same number.
// compatibilityToken must match exactly between client and host.
struct ChannelRegistration {
	uint32_t                structSize;
	uint32_t                flags;
	uint16_t                localChannelId;
	uint16_t                reserved;
	uint32_t                reserved2;
	uint64_t                compatibilityToken;
	HostMessageCallback     hostMessage;
	ClientMessageCallback   clientMessage;
	ConnectionStateCallback connectionState;
	void*                   userData;
};

struct ChannelInfo {
	uint32_t        structSize;
	uint32_t        flags;
	ChannelHandle   handle;
	uint64_t        ownerGeneration;
	uint64_t        compatibilityToken;
	uint16_t        localChannelId;
	uint16_t        reserved;
	ChannelState    state;
	ConnectionState connectionState;
	RejectionReason rejectionReason;
	uint32_t        reserved2;
};

inline constexpr uint32_t ConnectionEventSize             = static_cast<uint32_t>(sizeof(ConnectionEvent));
inline constexpr uint32_t ConnectionEventRequiredSize     = ConnectionEventSize;
inline constexpr uint32_t ChannelRegistrationSize         = static_cast<uint32_t>(sizeof(ChannelRegistration));
inline constexpr uint32_t ChannelRegistrationRequiredSize = static_cast<uint32_t>(offsetof(ChannelRegistration, userData) + sizeof(void*));
inline constexpr uint32_t ChannelInfoSize                 = static_cast<uint32_t>(sizeof(ChannelInfo));
inline constexpr uint32_t ChannelInfoRequiredSize         = static_cast<uint32_t>(offsetof(ChannelInfo, reserved2));

inline auto HasConnectionEventField(const ConnectionEvent* event, uint32_t fieldEndOffset) noexcept -> bool {
	return event != nullptr && event->structSize >= fieldEndOffset;
}

inline auto HasChannelRegistrationField(const ChannelRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasChannelInfoField(const ChannelInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

using RegisterChannelFn   = Result(__cdecl*)(const PluginContext* context, const ChannelRegistration* registration, ChannelHandle* handle) noexcept;
using UnregisterChannelFn = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle) noexcept;
using GetChannelInfoFn    = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle, ChannelInfo* info) noexcept;
// A TCP client may receive Busy until it processes the host's transport packet;
// retry later. Incompatible means the host uses a different D2RLoader transport
// and no plugin-network packet was sent.
using ConnectToHostFn     = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle) noexcept;
using SendToHostFn        = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle, uint16_t messageId, const void* data, uint32_t size) noexcept;
using SendToClientFn      = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle, PeerHandle peer, uint16_t messageId, const void* data, uint32_t size) noexcept;
// Resolves one connected peer to that player's authoritative server unit. Call
// this from a scheduled host game-thread callback. Clients cannot resolve peers.
using GetPeerPlayerFn     = Result(__cdecl*)(const PluginContext* context, ChannelHandle handle, PeerHandle peer, PlayerHandle* player) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(ChannelHandle) == sizeof(uint64_t));
static_assert(sizeof(PeerHandle) == sizeof(uint64_t));
static_assert(sizeof(ChannelState) == sizeof(uint32_t));
static_assert(sizeof(ConnectionState) == sizeof(uint32_t));
static_assert(sizeof(RejectionReason) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ConnectionEvent>);
static_assert(std::is_trivially_copyable_v<ConnectionEvent>);
static_assert(std::is_standard_layout_v<ChannelRegistration>);
static_assert(std::is_trivially_copyable_v<ChannelRegistration>);
static_assert(std::is_standard_layout_v<ChannelInfo>);
static_assert(std::is_trivially_copyable_v<ChannelInfo>);
static_assert(offsetof(ConnectionEvent, structSize) == 0);
static_assert(offsetof(ConnectionEvent, flags) == 4);
static_assert(offsetof(ConnectionEvent, channel) == 8);
static_assert(offsetof(ConnectionEvent, peer) == 16);
static_assert(offsetof(ConnectionEvent, state) == 24);
static_assert(offsetof(ConnectionEvent, reason) == 28);
static_assert(sizeof(ConnectionEvent) == 32);
static_assert(offsetof(ChannelRegistration, structSize) == 0);
static_assert(offsetof(ChannelRegistration, flags) == 4);
static_assert(offsetof(ChannelRegistration, localChannelId) == 8);
static_assert(offsetof(ChannelRegistration, reserved) == 10);
static_assert(offsetof(ChannelRegistration, reserved2) == 12);
static_assert(offsetof(ChannelRegistration, compatibilityToken) == 16);
static_assert(offsetof(ChannelRegistration, hostMessage) == 24);
static_assert(offsetof(ChannelRegistration, clientMessage) == 32);
static_assert(offsetof(ChannelRegistration, connectionState) == 40);
static_assert(offsetof(ChannelRegistration, userData) == 48);
static_assert(ChannelRegistrationRequiredSize == 56);
static_assert(sizeof(ChannelRegistration) == 56);
static_assert(offsetof(ChannelInfo, structSize) == 0);
static_assert(offsetof(ChannelInfo, flags) == 4);
static_assert(offsetof(ChannelInfo, handle) == 8);
static_assert(offsetof(ChannelInfo, ownerGeneration) == 16);
static_assert(offsetof(ChannelInfo, compatibilityToken) == 24);
static_assert(offsetof(ChannelInfo, localChannelId) == 32);
static_assert(offsetof(ChannelInfo, reserved) == 34);
static_assert(offsetof(ChannelInfo, state) == 36);
static_assert(offsetof(ChannelInfo, connectionState) == 40);
static_assert(offsetof(ChannelInfo, rejectionReason) == 44);
static_assert(offsetof(ChannelInfo, reserved2) == 48);
static_assert(ChannelInfoRequiredSize == 48);
static_assert(sizeof(ChannelInfo) == 56);

}

struct NetworkService {
	static constexpr ServiceId Id         = ServiceId::Network;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                     serviceSize;
	uint32_t                     serviceVersion;
	Network::RegisterChannelFn   registerChannel;
	Network::UnregisterChannelFn unregisterChannel;
	Network::GetChannelInfoFn    getChannelInfo;
	Network::ConnectToHostFn     connectToHost;
	Network::SendToHostFn        sendToHost;
	Network::SendToClientFn      sendToClient;
	Network::GetPeerPlayerFn     getPeerPlayer;
};

inline constexpr uint32_t NetworkServiceSize                      = static_cast<uint32_t>(sizeof(NetworkService));
inline constexpr uint32_t NetworkServiceRequiredSize              = static_cast<uint32_t>(offsetof(NetworkService, getPeerPlayer) + sizeof(Network::GetPeerPlayerFn));
inline constexpr uint32_t NetworkServiceRegisterChannelFieldEnd   = static_cast<uint32_t>(offsetof(NetworkService, registerChannel) + sizeof(Network::RegisterChannelFn));
inline constexpr uint32_t NetworkServiceUnregisterChannelFieldEnd = static_cast<uint32_t>(offsetof(NetworkService, unregisterChannel) + sizeof(Network::UnregisterChannelFn));
inline constexpr uint32_t NetworkServiceGetChannelInfoFieldEnd    = static_cast<uint32_t>(offsetof(NetworkService, getChannelInfo) + sizeof(Network::GetChannelInfoFn));
inline constexpr uint32_t NetworkServiceConnectToHostFieldEnd     = static_cast<uint32_t>(offsetof(NetworkService, connectToHost) + sizeof(Network::ConnectToHostFn));
inline constexpr uint32_t NetworkServiceSendToHostFieldEnd        = static_cast<uint32_t>(offsetof(NetworkService, sendToHost) + sizeof(Network::SendToHostFn));
inline constexpr uint32_t NetworkServiceSendToClientFieldEnd      = static_cast<uint32_t>(offsetof(NetworkService, sendToClient) + sizeof(Network::SendToClientFn));
inline constexpr uint32_t NetworkServiceGetPeerPlayerFieldEnd     = static_cast<uint32_t>(offsetof(NetworkService, getPeerPlayer) + sizeof(Network::GetPeerPlayerFn));

inline auto HasNetworkServiceField(const NetworkService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == NetworkService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<NetworkService>);
static_assert(std::is_trivially_copyable_v<NetworkService>);
static_assert(offsetof(NetworkService, registerChannel) == 8);
static_assert(offsetof(NetworkService, unregisterChannel) == 16);
static_assert(offsetof(NetworkService, getChannelInfo) == 24);
static_assert(offsetof(NetworkService, connectToHost) == 32);
static_assert(offsetof(NetworkService, sendToHost) == 40);
static_assert(offsetof(NetworkService, sendToClient) == 48);
static_assert(offsetof(NetworkService, getPeerPlayer) == 56);
static_assert(NetworkServiceRegisterChannelFieldEnd == 16);
static_assert(NetworkServiceUnregisterChannelFieldEnd == 24);
static_assert(NetworkServiceGetChannelInfoFieldEnd == 32);
static_assert(NetworkServiceConnectToHostFieldEnd == 40);
static_assert(NetworkServiceSendToHostFieldEnd == 48);
static_assert(NetworkServiceSendToClientFieldEnd == 56);
static_assert(NetworkServiceGetPeerPlayerFieldEnd == 64);
static_assert(NetworkServiceRequiredSize == 64);
static_assert(sizeof(NetworkService) == 64);

}
