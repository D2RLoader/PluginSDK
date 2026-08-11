#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Lifecycle {

using ListenerHandle = uint64_t;

inline constexpr ListenerHandle InvalidHandle = 0;

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
};

enum class GameplayEventKind : uint32_t {
	GameJoined        = 1,
	GameLeft          = 2,
	LocalPlayerReady  = 3,
	ActChanged        = 4,
	LevelChanged      = 5,
	PlayerResurrected = 6,
};

// revision starts at 1 and changes after every completed data-table load. The
// event pointer is valid only until the listener returns.
struct DataTablesLoadedEvent {
	uint32_t structSize;
	uint32_t flags;
	uint64_t revision;
};

using DataTablesLoadedCallback = void(__cdecl*)(const PluginContext* context, const DataTablesLoadedEvent* event, void* userData) noexcept;

// Register during D2RLoaderLoadPlugin. Listeners run synchronously on the game
// thread in registration order, after stock tables, plugin tables, and loader
// post-processing finish. Keep the callback short. A plugin may register more
// than one listener, and D2RLoader removes them all on unload.
struct DataTablesLoadedListener {
	uint32_t                 structSize;
	uint32_t                 flags;
	DataTablesLoadedCallback callback;
	void*                    userData;
};

struct GameplayEvent {
	uint32_t          structSize;
	uint32_t          flags;
	GameplayEventKind kind;
	uint32_t          playerId;
	uint64_t          sessionGeneration;
	int32_t           previousValue;
	int32_t           currentValue;
};

// Gameplay listeners run on the UI thread in registration order. ActChanged
// uses the zero-based act number in previousValue/currentValue. LevelChanged
// uses level ids. The other events leave both values at zero. A new
// sessionGeneration also invalidates player and item handles from the old game.

using GameplayEventCallback = void(__cdecl*)(const PluginContext* context, const GameplayEvent* event, void* userData) noexcept;

struct GameplayEventListener {
	uint32_t              structSize;
	uint32_t              flags;
	GameplayEventKind     kind;
	uint32_t              reserved;
	GameplayEventCallback callback;
	void*                 userData;
};

// Register one listener for each wanted event during D2RLoaderLoadPlugin.
// D2RLoader removes them on plugin unload.

inline constexpr uint32_t DataTablesLoadedEventSize            = static_cast<uint32_t>(sizeof(DataTablesLoadedEvent));
inline constexpr uint32_t DataTablesLoadedEventRequiredSize    = static_cast<uint32_t>(offsetof(DataTablesLoadedEvent, revision) + sizeof(uint64_t));
inline constexpr uint32_t DataTablesLoadedListenerSize         = static_cast<uint32_t>(sizeof(DataTablesLoadedListener));
inline constexpr uint32_t DataTablesLoadedListenerRequiredSize = static_cast<uint32_t>(offsetof(DataTablesLoadedListener, userData) + sizeof(void*));
inline constexpr uint32_t GameplayEventSize                    = static_cast<uint32_t>(sizeof(GameplayEvent));
inline constexpr uint32_t GameplayEventRequiredSize            = GameplayEventSize;
inline constexpr uint32_t GameplayEventListenerSize            = static_cast<uint32_t>(sizeof(GameplayEventListener));
inline constexpr uint32_t GameplayEventListenerRequiredSize    = GameplayEventListenerSize;

inline auto HasDataTablesLoadedEventField(const DataTablesLoadedEvent* event, uint32_t fieldEndOffset) noexcept -> bool {
	return event != nullptr && event->structSize >= fieldEndOffset;
}

inline auto HasDataTablesLoadedListenerField(const DataTablesLoadedListener* listener, uint32_t fieldEndOffset) noexcept -> bool {
	return listener != nullptr && listener->structSize >= fieldEndOffset;
}

inline auto HasGameplayEventField(const GameplayEvent* event, uint32_t fieldEndOffset) noexcept -> bool {
	return event != nullptr && event->structSize >= fieldEndOffset;
}

inline auto HasGameplayEventListenerField(const GameplayEventListener* listener, uint32_t fieldEndOffset) noexcept -> bool {
	return listener != nullptr && listener->structSize >= fieldEndOffset;
}

using RegisterDataTablesLoadedListenerFn   = Result(__cdecl*)(const PluginContext* context, const DataTablesLoadedListener* listener, ListenerHandle* handle) noexcept;
using UnregisterDataTablesLoadedListenerFn = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;
using RegisterGameplayEventListenerFn      = Result(__cdecl*)(const PluginContext* context, const GameplayEventListener* listener, ListenerHandle* handle) noexcept;
using UnregisterGameplayEventListenerFn    = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;

static_assert(sizeof(ListenerHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(GameplayEventKind) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<DataTablesLoadedEvent>);
static_assert(std::is_trivially_copyable_v<DataTablesLoadedEvent>);
static_assert(std::is_standard_layout_v<DataTablesLoadedListener>);
static_assert(std::is_trivially_copyable_v<DataTablesLoadedListener>);
static_assert(std::is_standard_layout_v<GameplayEvent> && std::is_trivially_copyable_v<GameplayEvent>);
static_assert(std::is_standard_layout_v<GameplayEventListener> && std::is_trivially_copyable_v<GameplayEventListener>);
static_assert(offsetof(DataTablesLoadedEvent, structSize) == 0);
static_assert(offsetof(DataTablesLoadedEvent, flags) == 4);
static_assert(offsetof(DataTablesLoadedEvent, revision) == 8);
static_assert(DataTablesLoadedEventRequiredSize == 16);
static_assert(sizeof(DataTablesLoadedEvent) == 16);
static_assert(offsetof(DataTablesLoadedListener, structSize) == 0);
static_assert(offsetof(DataTablesLoadedListener, flags) == 4);
static_assert(offsetof(DataTablesLoadedListener, callback) == 8);
static_assert(offsetof(DataTablesLoadedListener, userData) == 16);
static_assert(DataTablesLoadedListenerRequiredSize == 24);
static_assert(sizeof(DataTablesLoadedListener) == 24);
static_assert(sizeof(GameplayEvent) == 32);
static_assert(sizeof(GameplayEventListener) == 32);

}

struct LifecycleServiceV1 {
	uint32_t                                        serviceSize;
	uint32_t                                        serviceVersion;
	Lifecycle::RegisterDataTablesLoadedListenerFn   registerDataTablesLoadedListener;
	Lifecycle::UnregisterDataTablesLoadedListenerFn unregisterDataTablesLoadedListener;
	Lifecycle::RegisterGameplayEventListenerFn      registerGameplayEventListener;
	Lifecycle::UnregisterGameplayEventListenerFn    unregisterGameplayEventListener;
};

inline constexpr uint32_t LifecycleServiceV1Version      = 1;
inline constexpr uint32_t LifecycleServiceV1Size         = static_cast<uint32_t>(sizeof(LifecycleServiceV1));
inline constexpr uint32_t LifecycleServiceV1RequiredSize = LifecycleServiceV1Size;

inline auto HasLifecycleServiceV1Field(const LifecycleServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == LifecycleServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<LifecycleServiceV1>);
static_assert(std::is_trivially_copyable_v<LifecycleServiceV1>);
static_assert(offsetof(LifecycleServiceV1, serviceSize) == 0);
static_assert(offsetof(LifecycleServiceV1, serviceVersion) == 4);
static_assert(offsetof(LifecycleServiceV1, registerDataTablesLoadedListener) == 8);
static_assert(offsetof(LifecycleServiceV1, unregisterDataTablesLoadedListener) == 16);
static_assert(offsetof(LifecycleServiceV1, registerGameplayEventListener) == 24);
static_assert(offsetof(LifecycleServiceV1, unregisterGameplayEventListener) == 32);
static_assert(LifecycleServiceV1RequiredSize == 40);
static_assert(sizeof(LifecycleServiceV1) == 40);

}
