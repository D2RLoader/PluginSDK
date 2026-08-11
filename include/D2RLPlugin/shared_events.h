#pragma once

#include <D2RLPlugin/inventory.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace SharedEvents {

using ListenerHandle = uint64_t;

inline constexpr ListenerHandle InvalidHandle = 0;

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	Unavailable     = 3,
	NotFound        = 4,
	Busy            = 5,
	OwnerInactive   = 6,
	OwnerMismatch   = 7,
	CallbackFault   = 8,
};

enum class UiMessageAction : uint32_t {
	Continue = 0,
	Consume  = 1,
};

enum class ItemTooltipRegion : uint32_t {
	Description  = 0,
	Attributes   = 1,
	ActionFooter = 2,
};

enum class ItemTooltipPosition : uint32_t {
	Top         = 0,
	Bottom      = 1,
	AboveAnchor = 2,
	BelowAnchor = 3,
};

enum class ItemTooltipAnchor : uint32_t {
	None              = 0,
	Defense           = 1,
	Damage            = 2,
	ChanceToBlock     = 3,
	Durability        = 4,
	Quantity          = 5,
	RequiredStrength  = 6,
	RequiredDexterity = 7,
	RequiredLevel     = 8,
	AttackSpeed       = 9,
	Sockets           = 10,
};

enum class ItemTooltipFallback : uint32_t {
	Omit         = 0,
	RegionTop    = 1,
	RegionBottom = 2,
};

// Runs on the UI thread when the final item tooltip is about to be shown. text
// starts empty and receives only this listener's contribution, written in normal
// visual top-to-bottom order. Item-text contributions begin in neutral white so
// they do not inherit a native requirement or property color; an inline D2R
// color marker in the contribution can override it. capacity includes the
// trailing null byte. After writing, set length and keep text[length] equal to
// '\0'.
struct ItemTooltipEvent {
		uint32_t   structSize;
		uint32_t   flags;
		ItemHandle item;
		char*      text;
		uint32_t   length;
		uint32_t   capacity;
};

struct UiMessageEvent {
		uint32_t    structSize;
		uint32_t    flags;
		uint64_t    targetHash;
		uint64_t    commandHash;
		const char* target;
		const char* command;
		const char* text;
};

// UI message strings are borrowed and remain valid only for the callback.
// Returning Consume stops lower-priority listeners and normal panel handling.

using ItemTooltipCallback = void(__cdecl*)(const PluginContext* context, ItemTooltipEvent* event, void* userData) noexcept;
using UiMessageCallback   = UiMessageAction(__cdecl*)(const PluginContext* context, const UiMessageEvent* event, void* userData) noexcept;

struct ItemTooltipListener {
		uint32_t            structSize;
		uint32_t            flags;
		int32_t             priority;
		int32_t             slot;
		ItemTooltipRegion   region;
		ItemTooltipPosition position;
		ItemTooltipAnchor   anchor;
		ItemTooltipFallback fallback;
		ItemTooltipCallback callback;
		void*               userData;
};

struct UiMessageListener {
		uint32_t          structSize;
		uint32_t          flags;
		int32_t           priority;
		uint32_t          reserved;
		UiMessageCallback callback;
		void*             userData;
};

// Top and Bottom require anchor None. AboveAnchor and BelowAnchor require a
// named anchor and are not supported for ActionFooter. If that native field is
// absent, fallback selects the region edge or omits the contribution. Lower
// slots appear first visually at the same resolved position; priority and then
// registration order break ties. Higher priorities also run first. D2RLoader
// unregisters every listener when its owner plugin unloads. A listener may
// unregister itself from its callback.

inline constexpr uint32_t ItemTooltipEventSize            = static_cast<uint32_t>(sizeof(ItemTooltipEvent));
inline constexpr uint32_t ItemTooltipEventRequiredSize    = ItemTooltipEventSize;
inline constexpr uint32_t UiMessageEventSize              = static_cast<uint32_t>(sizeof(UiMessageEvent));
inline constexpr uint32_t UiMessageEventRequiredSize      = UiMessageEventSize;
inline constexpr uint32_t ItemTooltipListenerSize         = static_cast<uint32_t>(sizeof(ItemTooltipListener));
inline constexpr uint32_t ItemTooltipListenerRequiredSize = ItemTooltipListenerSize;
inline constexpr uint32_t UiMessageListenerSize           = static_cast<uint32_t>(sizeof(UiMessageListener));
inline constexpr uint32_t UiMessageListenerRequiredSize   = UiMessageListenerSize;

using RegisterItemTooltipListenerFn   = Result(__cdecl*)(const PluginContext* context, const ItemTooltipListener* listener, ListenerHandle* handle) noexcept;
using UnregisterItemTooltipListenerFn = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;
using RegisterUiMessageListenerFn     = Result(__cdecl*)(const PluginContext* context, const UiMessageListener* listener, ListenerHandle* handle) noexcept;
using UnregisterUiMessageListenerFn   = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(UiMessageAction) == sizeof(uint32_t));
static_assert(sizeof(ItemTooltipRegion) == sizeof(uint32_t));
static_assert(sizeof(ItemTooltipPosition) == sizeof(uint32_t));
static_assert(sizeof(ItemTooltipAnchor) == sizeof(uint32_t));
static_assert(sizeof(ItemTooltipFallback) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ItemTooltipEvent> && std::is_trivially_copyable_v<ItemTooltipEvent>);
static_assert(std::is_standard_layout_v<UiMessageEvent> && std::is_trivially_copyable_v<UiMessageEvent>);
static_assert(std::is_standard_layout_v<ItemTooltipListener> && std::is_trivially_copyable_v<ItemTooltipListener>);
static_assert(std::is_standard_layout_v<UiMessageListener> && std::is_trivially_copyable_v<UiMessageListener>);
static_assert(sizeof(ItemTooltipEvent) == 32);
static_assert(sizeof(UiMessageEvent) == 48);
static_assert(sizeof(ItemTooltipListener) == 48);
static_assert(sizeof(UiMessageListener) == 32);

}

struct SharedEventServiceV1 {
		uint32_t                                      serviceSize;
		uint32_t                                      serviceVersion;
		SharedEvents::RegisterItemTooltipListenerFn   registerItemTooltipListener;
		SharedEvents::UnregisterItemTooltipListenerFn unregisterItemTooltipListener;
		SharedEvents::RegisterUiMessageListenerFn     registerUiMessageListener;
		SharedEvents::UnregisterUiMessageListenerFn   unregisterUiMessageListener;
};

inline constexpr uint32_t SharedEventServiceV1Version      = 1;
inline constexpr uint32_t SharedEventServiceV1Size         = static_cast<uint32_t>(sizeof(SharedEventServiceV1));
inline constexpr uint32_t SharedEventServiceV1RequiredSize = SharedEventServiceV1Size;

inline auto HasSharedEventServiceV1Field(const SharedEventServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == SharedEventServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<SharedEventServiceV1> && std::is_trivially_copyable_v<SharedEventServiceV1>);
static_assert(sizeof(SharedEventServiceV1) == 40);

}
