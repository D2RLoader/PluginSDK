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

// Existing listeners receive these containers. Add SharedStash or Belt to a
// listener's containerMask to opt in to those activation events.
inline constexpr uint32_t DefaultContainerMask   = Items::ContainerBit(Items::ItemContainer::Inventory) | Items::ContainerBit(Items::ItemContainer::Cube)
                                                 | Items::ContainerBit(Items::ItemContainer::PersonalStash) | Items::ContainerBit(Items::ItemContainer::CustomPage);
inline constexpr uint32_t SupportedContainerMask = DefaultContainerMask | Items::ContainerBit(Items::ItemContainer::SharedStash) | Items::ContainerBit(Items::ItemContainer::Belt);

enum class Decision : uint32_t {
	Continue = 0,
	Consume  = 1,
};

// Runs on the UI thread before D2R handles an existing item activated through a
// proven inventory-grid widget. The handles belong to this listener's plugin.
// Consuming the event stops lower-priority listeners and the normal action.
// V1 covers inventory, Cube, personal stash, custom-page, shared-stash, and belt
// controls. SharedStash and Belt events are sent only to listeners that request
// them. Vendor, trade, corpse, ground, equipment, and cursor interactions are
// not emitted.
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
	// Zero keeps DefaultContainerMask. Before adding SharedStash or Belt, check
	// that the service contains supportedContainerMask and that it reports the bit.
	uint32_t                containerMask;
	uint32_t                reserved2;
};

using RegisterListenerFn   = Result(__cdecl*)(const PluginContext* context, const ItemInteractionListener* listener, ListenerHandle* handle) noexcept;
using UnregisterListenerFn = Result(__cdecl*)(const PluginContext* context, ListenerHandle handle) noexcept;

inline constexpr uint32_t ItemInteractionEventSize                     = static_cast<uint32_t>(sizeof(ItemInteractionEvent));
inline constexpr uint32_t ItemInteractionEventRequiredSize             = ItemInteractionEventSize;
inline constexpr uint32_t ItemInteractionListenerSize                  = static_cast<uint32_t>(sizeof(ItemInteractionListener));
inline constexpr uint32_t ItemInteractionListenerRequiredSize          = 32;
inline constexpr uint32_t ItemInteractionListenerContainerMaskFieldEnd = static_cast<uint32_t>(offsetof(ItemInteractionListener, containerMask) + sizeof(uint32_t));

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
static_assert(offsetof(ItemInteractionListener, containerMask) == 32);
static_assert(ItemInteractionListenerContainerMaskFieldEnd == 36);
static_assert(sizeof(ItemInteractionListener) == 40);

}

struct ItemInteractionService {
	static constexpr ServiceId Id         = ServiceId::ItemInteraction;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                               serviceSize;
	uint32_t                               serviceVersion;
	ItemInteractions::RegisterListenerFn   registerListener;
	ItemInteractions::UnregisterListenerFn unregisterListener;
	uint32_t                               supportedContainerMask;
	uint32_t                               reserved;
};

inline constexpr uint32_t ItemInteractionServiceSize                           = static_cast<uint32_t>(sizeof(ItemInteractionService));
inline constexpr uint32_t ItemInteractionServiceRequiredSize                   = 24;
inline constexpr uint32_t ItemInteractionServiceSupportedContainerMaskFieldEnd = static_cast<uint32_t>(offsetof(ItemInteractionService, supportedContainerMask) + sizeof(uint32_t));

inline auto HasItemInteractionServiceField(const ItemInteractionService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ItemInteractionService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ItemInteractionService> && std::is_trivially_copyable_v<ItemInteractionService>);
static_assert(offsetof(ItemInteractionService, registerListener) == 8);
static_assert(ItemInteractionServiceRequiredSize == 24);
static_assert(offsetof(ItemInteractionService, supportedContainerMask) == 24);
static_assert(ItemInteractionServiceSupportedContainerMaskFieldEnd == 28);
static_assert(sizeof(ItemInteractionService) == 32);

}
