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
	GameJoined         = 1,
	GameLeft           = 2,
	LocalPlayerReady   = 3,
	ActChanged         = 4,
	LevelChanged       = 5,
	PlayerResurrected  = 6,
	PlayerLevelChanged = 7,
	QuestCompleted     = 8,
};

// Values match D2R's native unit types. Invalid marks an identity that is not
// available, such as a death with no direct killer.
enum class UnitType : uint32_t {
	Player  = 0,
	Monster = 1,
	Object  = 2,
	Missile = 3,
	Item    = 4,
	Tile    = 5,
	Deleted = 6,
	Invalid = 0xFFFFFFFFU,
};

inline constexpr uint32_t InvalidUnitId = 0xFFFFFFFFU;

struct UnitIdentity {
	UnitType type;
	uint32_t id;
	uint32_t classId;
	uint32_t reserved;
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
	uint32_t          difficulty;
	uint32_t          questRecordId;
};

// Gameplay listeners run on the UI thread in registration order. ActChanged
// uses the zero-based act number in previousValue/currentValue. LevelChanged
// uses level ids. PlayerLevelChanged uses character levels. QuestCompleted is
// emitted when PrimaryGoalDone changes from zero to one; difficulty is 0 for
// Normal, 1 for Nightmare, or 2 for Hell, and questRecordId is the zero-based
// quest state row. Existing completions are baselined when the player becomes
// ready. Initial ActChanged/LevelChanged events use -1 as previousValue. Fields
// not used by an event are zero. A new
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

struct MonsterDeathEvent {
	uint32_t     structSize;
	uint32_t     flags;
	uint32_t     gameId;
	uint32_t     difficulty;
	int32_t      levelId;
	uint32_t     reserved;
	UnitIdentity monster;
	UnitIdentity killer;
};

using MonsterDeathCallback = void(__cdecl*)(const PluginContext* context, const MonsterDeathEvent* event, void* userData) noexcept;

struct MonsterDeathListener {
	uint32_t             structSize;
	uint32_t             flags;
	MonsterDeathCallback callback;
	void*                userData;
};

// Monster-death listeners run synchronously on the authoritative server thread
// after D2R processes the normal drop. They are observation-only and should
// return quickly. monster is always the dead monster. killer is D2R's direct
// killer and may be a player, monster, missile, or another unit type. Its type
// is Invalid and its ids are InvalidUnitId when no killer is available. levelId
// is -1 when the monster has no level. Event pointers are valid only until the
// listener returns.

inline constexpr uint32_t DataTablesLoadedEventSize            = static_cast<uint32_t>(sizeof(DataTablesLoadedEvent));
inline constexpr uint32_t DataTablesLoadedEventRequiredSize    = static_cast<uint32_t>(offsetof(DataTablesLoadedEvent, revision) + sizeof(uint64_t));
inline constexpr uint32_t DataTablesLoadedListenerSize         = static_cast<uint32_t>(sizeof(DataTablesLoadedListener));
inline constexpr uint32_t DataTablesLoadedListenerRequiredSize = static_cast<uint32_t>(offsetof(DataTablesLoadedListener, userData) + sizeof(void*));
inline constexpr uint32_t GameplayEventSize                    = static_cast<uint32_t>(sizeof(GameplayEvent));
inline constexpr uint32_t GameplayEventRequiredSize            = GameplayEventSize;
inline constexpr uint32_t GameplayEventListenerSize            = static_cast<uint32_t>(sizeof(GameplayEventListener));
inline constexpr uint32_t GameplayEventListenerRequiredSize    = GameplayEventListenerSize;
inline constexpr uint32_t MonsterDeathEventSize                = static_cast<uint32_t>(sizeof(MonsterDeathEvent));
inline constexpr uint32_t MonsterDeathEventRequiredSize        = MonsterDeathEventSize;
inline constexpr uint32_t MonsterDeathListenerSize             = static_cast<uint32_t>(sizeof(MonsterDeathListener));
inline constexpr uint32_t MonsterDeathListenerRequiredSize     = MonsterDeathListenerSize;

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

inline auto HasMonsterDeathEventField(const MonsterDeathEvent* event, uint32_t fieldEndOffset) noexcept -> bool {
	return event != nullptr && event->structSize >= fieldEndOffset;
}

inline auto HasMonsterDeathListenerField(const MonsterDeathListener* listener, uint32_t fieldEndOffset) noexcept -> bool {
	return listener != nullptr && listener->structSize >= fieldEndOffset;
}

using RegisterDataTablesLoadedListenerFn   = Result(__cdecl*)(const PluginContext* context, const DataTablesLoadedListener* listener, ListenerHandle* handle) noexcept;
using UnregisterDataTablesLoadedListenerFn = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;
using RegisterGameplayEventListenerFn      = Result(__cdecl*)(const PluginContext* context, const GameplayEventListener* listener, ListenerHandle* handle) noexcept;
using UnregisterGameplayEventListenerFn    = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;
using RegisterMonsterDeathListenerFn       = Result(__cdecl*)(const PluginContext* context, const MonsterDeathListener* listener, ListenerHandle* handle) noexcept;
using UnregisterMonsterDeathListenerFn     = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;

static_assert(sizeof(ListenerHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(GameplayEventKind) == sizeof(uint32_t));
static_assert(sizeof(UnitType) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<DataTablesLoadedEvent>);
static_assert(std::is_trivially_copyable_v<DataTablesLoadedEvent>);
static_assert(std::is_standard_layout_v<DataTablesLoadedListener>);
static_assert(std::is_trivially_copyable_v<DataTablesLoadedListener>);
static_assert(std::is_standard_layout_v<GameplayEvent> && std::is_trivially_copyable_v<GameplayEvent>);
static_assert(std::is_standard_layout_v<GameplayEventListener> && std::is_trivially_copyable_v<GameplayEventListener>);
static_assert(std::is_standard_layout_v<UnitIdentity> && std::is_trivially_copyable_v<UnitIdentity>);
static_assert(std::is_standard_layout_v<MonsterDeathEvent> && std::is_trivially_copyable_v<MonsterDeathEvent>);
static_assert(std::is_standard_layout_v<MonsterDeathListener> && std::is_trivially_copyable_v<MonsterDeathListener>);
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
static_assert(sizeof(GameplayEvent) == 40);
static_assert(sizeof(GameplayEventListener) == 32);
static_assert(sizeof(UnitIdentity) == 16);
static_assert(offsetof(MonsterDeathEvent, monster) == 24);
static_assert(offsetof(MonsterDeathEvent, killer) == 40);
static_assert(sizeof(MonsterDeathEvent) == 56);
static_assert(sizeof(MonsterDeathListener) == 24);

}

struct LifecycleService {
	static constexpr ServiceId Id         = ServiceId::Lifecycle;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                                        serviceSize;
	uint32_t                                        serviceVersion;
	Lifecycle::RegisterDataTablesLoadedListenerFn   registerDataTablesLoadedListener;
	Lifecycle::UnregisterDataTablesLoadedListenerFn unregisterDataTablesLoadedListener;
	Lifecycle::RegisterGameplayEventListenerFn      registerGameplayEventListener;
	Lifecycle::UnregisterGameplayEventListenerFn    unregisterGameplayEventListener;
	Lifecycle::RegisterMonsterDeathListenerFn       registerMonsterDeathListener;
	Lifecycle::UnregisterMonsterDeathListenerFn     unregisterMonsterDeathListener;
};

inline constexpr uint32_t LifecycleServiceSize         = static_cast<uint32_t>(sizeof(LifecycleService));
inline constexpr uint32_t LifecycleServiceRequiredSize = LifecycleServiceSize;

inline auto HasLifecycleServiceField(const LifecycleService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == LifecycleService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<LifecycleService>);
static_assert(std::is_trivially_copyable_v<LifecycleService>);
static_assert(offsetof(LifecycleService, serviceSize) == 0);
static_assert(offsetof(LifecycleService, serviceVersion) == 4);
static_assert(offsetof(LifecycleService, registerDataTablesLoadedListener) == 8);
static_assert(offsetof(LifecycleService, unregisterDataTablesLoadedListener) == 16);
static_assert(offsetof(LifecycleService, registerGameplayEventListener) == 24);
static_assert(offsetof(LifecycleService, unregisterGameplayEventListener) == 32);
static_assert(offsetof(LifecycleService, registerMonsterDeathListener) == 40);
static_assert(offsetof(LifecycleService, unregisterMonsterDeathListener) == 48);
static_assert(LifecycleServiceRequiredSize == 56);
static_assert(sizeof(LifecycleService) == 56);

}
