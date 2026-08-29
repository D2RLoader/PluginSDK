#pragma once

#include <D2RLPlugin/handles.h>
#include <D2RLPlugin/item.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace ItemInteractions {

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

enum class Action : uint32_t {
	Activate = 1,
};

enum class InputSource : uint32_t {
	Unknown       = 0,
	KeyboardMouse = 1,
	Controller    = 2,
};

enum class Modifier : uint32_t {
	Shift   = 1U << 0U,
	Control = 1U << 1U,
	Alt     = 1U << 2U,
};

constexpr auto ModifierBit(Modifier modifier) noexcept -> uint32_t {
	return static_cast<uint32_t>(modifier);
}

inline constexpr uint32_t AllModifiers = ModifierBit(Modifier::Shift) | ModifierBit(Modifier::Control) | ModifierBit(Modifier::Alt);

enum class Decision : uint32_t {
	Continue = 0,
	Consume  = 1,
};

// Runs on the UI thread before D2R handles an existing item activated through a
// proven inventory-grid widget. The handles belong to this listener's plugin.
// Consuming the event stops lower-priority listeners and the normal action.
// V1 covers inventory, Cube, personal stash, and custom-page grids. Vendor,
// trade, corpse, ground, equipment, cursor, belt, and shared-stash interactions
// are not emitted yet.
struct ItemInteractionEvent {
		uint32_t             structSize;
		uint32_t             flags;
		PlayerHandle         player;
		ItemHandle           item;
		Action               action;
		InputSource          inputSource;
		uint32_t             modifiers;
		Items::ItemContainer container;
		int32_t              inventoryPage;
		int32_t              cellX;
		int32_t              cellY;
};

using ItemInteractionCallback = Decision(__cdecl*)(const PluginContext* context, const ItemInteractionEvent* event, void* userData) noexcept;

struct ItemInteractionListener {
		uint32_t                structSize;
		uint32_t                flags;
		int32_t                 priority;
		uint32_t                reserved;
		ItemInteractionCallback callback;
		void*                   userData;
};

using RegisterListenerFn   = Result(__cdecl*)(const PluginContext* context, const ItemInteractionListener* listener, ListenerHandle* handle) noexcept;
using UnregisterListenerFn = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;

inline constexpr uint32_t ItemInteractionEventSize            = static_cast<uint32_t>(sizeof(ItemInteractionEvent));
inline constexpr uint32_t ItemInteractionEventRequiredSize    = ItemInteractionEventSize;
inline constexpr uint32_t ItemInteractionListenerSize         = static_cast<uint32_t>(sizeof(ItemInteractionListener));
inline constexpr uint32_t ItemInteractionListenerRequiredSize = ItemInteractionListenerSize;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(Action) == sizeof(uint32_t));
static_assert(sizeof(InputSource) == sizeof(uint32_t));
static_assert(sizeof(Modifier) == sizeof(uint32_t));
static_assert(sizeof(Decision) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ItemInteractionEvent> && std::is_trivially_copyable_v<ItemInteractionEvent>);
static_assert(std::is_standard_layout_v<ItemInteractionListener> && std::is_trivially_copyable_v<ItemInteractionListener>);
static_assert(offsetof(ItemInteractionEvent, player) == 8);
static_assert(offsetof(ItemInteractionEvent, item) == 16);
static_assert(ItemInteractionEventRequiredSize == 56);
static_assert(sizeof(ItemInteractionEvent) == 56);
static_assert(offsetof(ItemInteractionListener, callback) == 16);
static_assert(ItemInteractionListenerRequiredSize == 32);
static_assert(sizeof(ItemInteractionListener) == 32);

}

struct ItemInteractionServiceV1 {
		uint32_t                               serviceSize;
		uint32_t                               serviceVersion;
		ItemInteractions::RegisterListenerFn   registerListener;
		ItemInteractions::UnregisterListenerFn unregisterListener;
};

inline constexpr uint32_t ItemInteractionServiceV1Version      = 1;
inline constexpr uint32_t ItemInteractionServiceV1Size         = static_cast<uint32_t>(sizeof(ItemInteractionServiceV1));
inline constexpr uint32_t ItemInteractionServiceV1RequiredSize = ItemInteractionServiceV1Size;

inline auto HasItemInteractionServiceV1Field(const ItemInteractionServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ItemInteractionServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ItemInteractionServiceV1> && std::is_trivially_copyable_v<ItemInteractionServiceV1>);
static_assert(offsetof(ItemInteractionServiceV1, registerListener) == 8);
static_assert(ItemInteractionServiceV1RequiredSize == 24);
static_assert(sizeof(ItemInteractionServiceV1) == 24);

}
