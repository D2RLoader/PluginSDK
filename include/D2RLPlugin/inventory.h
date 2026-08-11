#pragma once

#include <D2RLPlugin/handles.h>
#include <D2RLPlugin/item.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Inventory {

using PageRegistrationHandle = uint64_t;

inline constexpr uint64_t InvalidHandle = 0;

// Player and item handles belong to the plugin that received them. They are not
// game pointers. A new game session makes them stale, and every item call checks
// the runtime unit id and item seed again.

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
	PolicyRejected  = 11,
};

enum class IterationAction : uint32_t {
	Continue = 0,
	Stop     = 1,
};

enum class PageRegistrationState : uint32_t {
	Unknown = 0,
	Staged  = 1,
	Active  = 2,
	Closing = 3,
};

enum class PageConfigurationState : uint32_t {
	Unknown    = 0,
	Reserved   = 1,
	Configured = 2,
	Closing    = 3,
	Faulted    = 4,
};

enum class LocalPlayerPageMoveKind : uint32_t {
	Unknown      = 0,
	CursorToPage = 1,
	PageToCursor = 2,
};

enum class ItemPlacementReason : uint32_t {
	Unknown          = 0,
	LocalMove        = 1,
	SavedItemRestore = 2,
};

enum class ItemPolicyDecision : uint32_t {
	Unknown    = 0,
	Allow      = 1,
	Reject     = 2,
	RemoveOnly = 3,
};

enum class PlayerPageCharmPolicy : uint32_t {
	Unknown                     = 0,
	StockInventoryOnly          = 1,
	PlayerPageOnly              = 2,
	StockInventoryAndPlayerPage = 3,
};

// Selects one of D2R's native stored-item tint colors for charms whose bonuses
// are inactive. Default uses the loader's configured default, currently Yellow.
enum class PlayerPageInactiveCharmTint : uint32_t {
	Default = 0,
	Red     = 1,
	Green   = 2,
	Blue    = 3,
	Yellow  = 4,
	PaleRed = 5,
};

inline constexpr uint32_t PlayerPageInactiveCharmTintMask = 0x00000007;

// A D2 item-type code contains one through four ASCII characters. Short TXT
// codes are zero-padded to four bytes. Use MakeItemTypeFourCC when comparing
// one with a callback value.
struct ItemTypeFourCC {
	char value[4];
};

constexpr auto MakeItemTypeFourCC(char first, char second = 0, char third = 0, char fourth = 0) noexcept -> ItemTypeFourCC {
	return {
		{ first, second, third, fourth }
	};
}

template <size_t Size>
constexpr auto MakeItemTypeFourCC(const char (&value)[Size]) noexcept -> ItemTypeFourCC {
	static_assert(Size >= 2 && Size <= 5, "Item type text must contain one through four characters.");
	ItemTypeFourCC itemType {};
	for (size_t index = 0; index + 1 < Size; ++index) {
		itemType.value[index] = value[index];
	}
	return itemType;
}

constexpr auto ItemTypeFourCCEquals(ItemTypeFourCC lhs, ItemTypeFourCC rhs) noexcept -> bool {
	return lhs.value[0] == rhs.value[0] && lhs.value[1] == rhs.value[1] && lhs.value[2] == rhs.value[2] && lhs.value[3] == rhs.value[3];
}

// D2RLoader owns saved-item placement for this page. It restores supported items
// or fails the load before stock code can silently discard them.
inline constexpr uint32_t PlayerPageConfigurationInfoFlagSaveLoadProtected = 1U << 0U;

struct PlayerPageRegistration {
	uint32_t    structSize;
	uint32_t    flags;
	const char* logicalId;
};

struct PageRegistrationInfo {
	uint32_t               structSize;
	uint32_t               flags;
	PageRegistrationHandle handle;
	uint64_t               ownerGeneration;
	PageRegistrationState  state;
	uint32_t               reserved;
};

struct PlayerPageConfiguration {
	uint32_t structSize;
	uint32_t flags;
	// D2S stores each item coordinate in four bits, so v1 accepts dimensions
	// from 1 through 16. Once the native grid has been used, a different size
	// returns Busy. Restart before changing it.
	uint32_t width;
	uint32_t height;
};

struct PlayerPageConfigurationInfo {
	uint32_t               structSize;
	uint32_t               flags;
	PageRegistrationHandle handle;
	uint64_t               configurationRevision;
	uint32_t               width;
	uint32_t               height;
	PageConfigurationState state;
	uint32_t               reserved;
};

struct LocalPlayerPageMoveRequest {
	uint32_t                structSize;
	uint32_t                flags;
	LocalPlayerPageMoveKind kind;
	uint32_t                x;
	uint32_t                y;
};

// The loader owns this request and its matchingItemTypes array. Both contain
// copied data and are valid until the synchronous callback returns. The array
// includes the item's exact type and the parent types that D2 considers it to
// match. Its order is not part of the ABI.
struct ItemPolicyRequest {
	uint32_t               structSize;
	uint32_t               flags;
	PageRegistrationHandle page;
	ItemPlacementReason    reason;
	uint32_t               matchingItemTypeCount;
	const ItemTypeFourCC*  matchingItemTypes;
};

using ItemPolicyCallback = ItemPolicyDecision(__cdecl*)(const PluginContext* context, const ItemPolicyRequest* request, void* userData) noexcept;

// Configure one registered page. D2RLoader copies callback and userData before
// returning. Pass a null callback to clear the policy and restore the default
// allow-all behavior. flags is zero in v1.
//
// LocalMove continues only for Allow. Reject and RemoveOnly both block a new
// item. SavedItemRestore uses RemoveOnly for recovery: keep an invalid saved
// item, but only let the player remove it. Unknown and invalid results fail
// closed.
struct PlayerPageItemPolicy {
	uint32_t               structSize;
	uint32_t               flags;
	PageRegistrationHandle page;
	ItemPolicyCallback     callback;
	void*                  userData;
};

// Choose which carried pages activate charm bonuses. Configure this while the
// page is still staged during plugin load. It is separate from item admission.
// The default is StockInventoryOnly; Unknown is invalid. The low three flags
// bits may hold a PlayerPageInactiveCharmTint. Leave every other bit zero.
struct PlayerPageCharmPolicyConfiguration {
	uint32_t               structSize;
	uint32_t               flags;
	PageRegistrationHandle page;
	PlayerPageCharmPolicy  policy;
};

struct ItemFilter {
	uint32_t structSize;
	uint32_t flags;
	uint32_t containerMask;
	uint32_t reserved;
};

using ItemCallback = IterationAction(__cdecl*)(const PluginContext* context, const Items::ItemInfo* item, void* userData) noexcept;

inline constexpr uint32_t PlayerPageRegistrationSize                     = static_cast<uint32_t>(sizeof(PlayerPageRegistration));
inline constexpr uint32_t PlayerPageRegistrationRequiredSize             = static_cast<uint32_t>(offsetof(PlayerPageRegistration, logicalId) + sizeof(const char*));
inline constexpr uint32_t PageRegistrationInfoSize                       = static_cast<uint32_t>(sizeof(PageRegistrationInfo));
inline constexpr uint32_t PageRegistrationInfoRequiredSize               = static_cast<uint32_t>(offsetof(PageRegistrationInfo, reserved));
inline constexpr uint32_t PlayerPageConfigurationSize                    = static_cast<uint32_t>(sizeof(PlayerPageConfiguration));
inline constexpr uint32_t PlayerPageConfigurationRequiredSize            = static_cast<uint32_t>(offsetof(PlayerPageConfiguration, height) + sizeof(uint32_t));
inline constexpr uint32_t PlayerPageConfigurationInfoSize                = static_cast<uint32_t>(sizeof(PlayerPageConfigurationInfo));
inline constexpr uint32_t PlayerPageConfigurationInfoRequiredSize        = static_cast<uint32_t>(offsetof(PlayerPageConfigurationInfo, reserved));
inline constexpr uint32_t LocalPlayerPageMoveRequestSize                 = static_cast<uint32_t>(sizeof(LocalPlayerPageMoveRequest));
inline constexpr uint32_t LocalPlayerPageMoveRequestRequiredSize         = static_cast<uint32_t>(offsetof(LocalPlayerPageMoveRequest, y) + sizeof(uint32_t));
inline constexpr uint32_t ItemPolicyRequestSize                          = static_cast<uint32_t>(sizeof(ItemPolicyRequest));
inline constexpr uint32_t ItemPolicyRequestRequiredSize                  = static_cast<uint32_t>(offsetof(ItemPolicyRequest, matchingItemTypes) + sizeof(const ItemTypeFourCC*));
inline constexpr uint32_t PlayerPageItemPolicySize                       = static_cast<uint32_t>(sizeof(PlayerPageItemPolicy));
inline constexpr uint32_t PlayerPageItemPolicyRequiredSize               = static_cast<uint32_t>(offsetof(PlayerPageItemPolicy, userData) + sizeof(void*));
inline constexpr uint32_t PlayerPageCharmPolicyConfigurationSize         = static_cast<uint32_t>(sizeof(PlayerPageCharmPolicyConfiguration));
inline constexpr uint32_t PlayerPageCharmPolicyConfigurationRequiredSize = static_cast<uint32_t>(offsetof(PlayerPageCharmPolicyConfiguration, policy) + sizeof(PlayerPageCharmPolicy));
inline constexpr uint32_t ItemFilterSize                                 = static_cast<uint32_t>(sizeof(ItemFilter));
inline constexpr uint32_t ItemFilterRequiredSize                         = ItemFilterSize;

inline auto HasPlayerPageRegistrationField(const PlayerPageRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasPageRegistrationInfoField(const PageRegistrationInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

inline auto HasPlayerPageConfigurationField(const PlayerPageConfiguration* configuration, uint32_t fieldEndOffset) noexcept -> bool {
	return configuration != nullptr && configuration->structSize >= fieldEndOffset;
}

inline auto HasPlayerPageConfigurationInfoField(const PlayerPageConfigurationInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

inline auto HasLocalPlayerPageMoveRequestField(const LocalPlayerPageMoveRequest* request, uint32_t fieldEndOffset) noexcept -> bool {
	return request != nullptr && request->structSize >= fieldEndOffset;
}

inline auto HasItemPolicyRequestField(const ItemPolicyRequest* request, uint32_t fieldEndOffset) noexcept -> bool {
	return request != nullptr && request->structSize >= fieldEndOffset;
}

inline auto HasPlayerPageItemPolicyField(const PlayerPageItemPolicy* policy, uint32_t fieldEndOffset) noexcept -> bool {
	return policy != nullptr && policy->structSize >= fieldEndOffset;
}

inline auto HasPlayerPageCharmPolicyConfigurationField(const PlayerPageCharmPolicyConfiguration* configuration, uint32_t fieldEndOffset) noexcept -> bool {
	return configuration != nullptr && configuration->structSize >= fieldEndOffset;
}

inline auto HasItemFilterField(const ItemFilter* filter, uint32_t fieldEndOffset) noexcept -> bool {
	return filter != nullptr && filter->structSize >= fieldEndOffset;
}

inline auto ItemPolicyRequestMatchesType(const ItemPolicyRequest* request, ItemTypeFourCC itemType) noexcept -> bool {
	if (!HasItemPolicyRequestField(request, ItemPolicyRequestRequiredSize) || request->matchingItemTypes == nullptr) {
		return false;
	}

	for (uint32_t index = 0; index < request->matchingItemTypeCount; ++index) {
		if (ItemTypeFourCCEquals(request->matchingItemTypes[index], itemType)) {
			return true;
		}
	}

	return false;
}

using RegisterPlayerPageFn             = Result(__cdecl*)(const PluginContext* context, const PlayerPageRegistration* registration, PageRegistrationHandle* handle) noexcept;
using UnregisterPlayerPageFn           = Result(__cdecl*)(const PluginContext* context, PageRegistrationHandle handle) noexcept;
using GetPageRegistrationInfoFn        = Result(__cdecl*)(const PluginContext* context, PageRegistrationHandle handle, PageRegistrationInfo* info) noexcept;
using ConfigurePlayerPageFn            = Result(__cdecl*)(const PluginContext* context, PageRegistrationHandle handle, const PlayerPageConfiguration* configuration) noexcept;
using GetPlayerPageConfigurationInfoFn = Result(__cdecl*)(const PluginContext* context, PageRegistrationHandle handle, PlayerPageConfigurationInfo* info) noexcept;
// Moves one item between the cursor and the authoritative local player's page.
// It resolves that player on every call and never exposes native player, item,
// page, or grid pointers. Native item dimensions and several top-level items are
// supported.
// CursorToPage treats x/y as the new item's top-left cell. PageToCursor accepts
// any occupied cell and resolves the item's stored top-left cell internally.
// Call it synchronously from a D2RLoader game-thread callback. V1 does not
// support inventory mutation from a worker thread.
using ExecuteLocalPlayerMoveFn         = Result(__cdecl*)(const PluginContext* context, PageRegistrationHandle handle, const LocalPlayerPageMoveRequest* request) noexcept;
// Looks up a zero-padded code in the active RotW ItemTypes bank. Success writes
// true or false to known without exposing a row number or native table pointer.
using IsItemTypeCodeKnownFn            = Result(__cdecl*)(const PluginContext* context, ItemTypeFourCC itemType, bool* known) noexcept;
using ConfigurePlayerPageItemPolicyFn  = Result(__cdecl*)(const PluginContext* context, const PlayerPageItemPolicy* policy) noexcept;
using ConfigurePlayerPageCharmPolicyFn = Result(__cdecl*)(const PluginContext* context, const PlayerPageCharmPolicyConfiguration* configuration) noexcept;
using GetLocalPlayerFn                 = Result(__cdecl*)(const PluginContext* context, PlayerHandle* player) noexcept;
using GetCursorItemFn                  = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, ItemHandle* item) noexcept;
using GetEquippedItemFn                = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, int32_t bodyLocation, ItemHandle* item) noexcept;
using ForEachInventoryItemFn           = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, const ItemFilter* filter, ItemCallback callback, void* userData) noexcept;

// Capture the local player on the UI thread. Use its handle for reads on that
// thread or in a scheduled game-thread callback. Enumeration is synchronous;
// every ItemInfo passed to the callback is a copy.
//
static_assert(sizeof(PageRegistrationHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(PageRegistrationState) == sizeof(uint32_t));
static_assert(sizeof(PageConfigurationState) == sizeof(uint32_t));
static_assert(sizeof(LocalPlayerPageMoveKind) == sizeof(uint32_t));
static_assert(sizeof(ItemPlacementReason) == sizeof(uint32_t));
static_assert(sizeof(ItemPolicyDecision) == sizeof(uint32_t));
static_assert(sizeof(IterationAction) == sizeof(uint32_t));
static_assert(sizeof(PlayerPageCharmPolicy) == sizeof(uint32_t));
static_assert(sizeof(PlayerPageInactiveCharmTint) == sizeof(uint32_t));
static_assert(sizeof(ItemTypeFourCC) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ItemTypeFourCC>);
static_assert(std::is_trivially_copyable_v<ItemTypeFourCC>);
static_assert(ItemTypeFourCCEquals(MakeItemTypeFourCC("char"), MakeItemTypeFourCC('c', 'h', 'a', 'r')));
static_assert(ItemTypeFourCCEquals(MakeItemTypeFourCC("gem"), MakeItemTypeFourCC('g', 'e', 'm')));
static_assert(std::is_standard_layout_v<PlayerPageRegistration>);
static_assert(std::is_trivially_copyable_v<PlayerPageRegistration>);
static_assert(std::is_standard_layout_v<PageRegistrationInfo>);
static_assert(std::is_trivially_copyable_v<PageRegistrationInfo>);
static_assert(std::is_standard_layout_v<PlayerPageConfiguration>);
static_assert(std::is_trivially_copyable_v<PlayerPageConfiguration>);
static_assert(std::is_standard_layout_v<PlayerPageConfigurationInfo>);
static_assert(std::is_trivially_copyable_v<PlayerPageConfigurationInfo>);
static_assert(std::is_standard_layout_v<LocalPlayerPageMoveRequest>);
static_assert(std::is_trivially_copyable_v<LocalPlayerPageMoveRequest>);
static_assert(std::is_standard_layout_v<ItemPolicyRequest>);
static_assert(std::is_trivially_copyable_v<ItemPolicyRequest>);
static_assert(std::is_standard_layout_v<PlayerPageItemPolicy>);
static_assert(std::is_trivially_copyable_v<PlayerPageItemPolicy>);
static_assert(std::is_standard_layout_v<PlayerPageCharmPolicyConfiguration>);
static_assert(std::is_trivially_copyable_v<PlayerPageCharmPolicyConfiguration>);
static_assert(std::is_standard_layout_v<ItemFilter> && std::is_trivially_copyable_v<ItemFilter>);
static_assert(offsetof(PlayerPageRegistration, logicalId) == 8);
static_assert(sizeof(PlayerPageRegistration) == 16);
static_assert(offsetof(PageRegistrationInfo, handle) == 8);
static_assert(offsetof(PageRegistrationInfo, ownerGeneration) == 16);
static_assert(offsetof(PageRegistrationInfo, state) == 24);
static_assert(offsetof(PageRegistrationInfo, reserved) == 28);
static_assert(PageRegistrationInfoRequiredSize == 28);
static_assert(sizeof(PageRegistrationInfo) == 32);
static_assert(offsetof(PlayerPageConfiguration, width) == 8);
static_assert(offsetof(PlayerPageConfiguration, height) == 12);
static_assert(sizeof(PlayerPageConfiguration) == 16);
static_assert(offsetof(PlayerPageConfigurationInfo, handle) == 8);
static_assert(offsetof(PlayerPageConfigurationInfo, configurationRevision) == 16);
static_assert(offsetof(PlayerPageConfigurationInfo, width) == 24);
static_assert(offsetof(PlayerPageConfigurationInfo, height) == 28);
static_assert(offsetof(PlayerPageConfigurationInfo, state) == 32);
static_assert(offsetof(PlayerPageConfigurationInfo, reserved) == 36);
static_assert(PlayerPageConfigurationInfoRequiredSize == 36);
static_assert(sizeof(PlayerPageConfigurationInfo) == 40);
static_assert(offsetof(LocalPlayerPageMoveRequest, structSize) == 0);
static_assert(offsetof(LocalPlayerPageMoveRequest, flags) == 4);
static_assert(offsetof(LocalPlayerPageMoveRequest, kind) == 8);
static_assert(offsetof(LocalPlayerPageMoveRequest, x) == 12);
static_assert(offsetof(LocalPlayerPageMoveRequest, y) == 16);
static_assert(LocalPlayerPageMoveRequestRequiredSize == 20);
static_assert(sizeof(LocalPlayerPageMoveRequest) == 20);
static_assert(offsetof(ItemPolicyRequest, structSize) == 0);
static_assert(offsetof(ItemPolicyRequest, flags) == 4);
static_assert(offsetof(ItemPolicyRequest, page) == 8);
static_assert(offsetof(ItemPolicyRequest, reason) == 16);
static_assert(offsetof(ItemPolicyRequest, matchingItemTypeCount) == 20);
static_assert(offsetof(ItemPolicyRequest, matchingItemTypes) == 24);
static_assert(ItemPolicyRequestRequiredSize == 32);
static_assert(sizeof(ItemPolicyRequest) == 32);
static_assert(offsetof(PlayerPageItemPolicy, structSize) == 0);
static_assert(offsetof(PlayerPageItemPolicy, flags) == 4);
static_assert(offsetof(PlayerPageItemPolicy, page) == 8);
static_assert(offsetof(PlayerPageItemPolicy, callback) == 16);
static_assert(offsetof(PlayerPageItemPolicy, userData) == 24);
static_assert(PlayerPageItemPolicyRequiredSize == 32);
static_assert(sizeof(PlayerPageItemPolicy) == 32);
static_assert(offsetof(PlayerPageCharmPolicyConfiguration, structSize) == 0);
static_assert(offsetof(PlayerPageCharmPolicyConfiguration, flags) == 4);
static_assert(offsetof(PlayerPageCharmPolicyConfiguration, page) == 8);
static_assert(offsetof(PlayerPageCharmPolicyConfiguration, policy) == 16);
static_assert(PlayerPageCharmPolicyConfigurationRequiredSize == 20);
static_assert(sizeof(PlayerPageCharmPolicyConfiguration) == 24);
static_assert(sizeof(ItemFilter) == 16);

}

struct InventoryServiceV1 {
	uint32_t                                    serviceSize;
	uint32_t                                    serviceVersion;
	Inventory::RegisterPlayerPageFn             registerPlayerPage;
	Inventory::UnregisterPlayerPageFn           unregisterPlayerPage;
	Inventory::GetPageRegistrationInfoFn        getRegistrationInfo;
	Inventory::ConfigurePlayerPageFn            configurePlayerPage;
	Inventory::GetPlayerPageConfigurationInfoFn getPlayerPageConfigurationInfo;
	Inventory::ExecuteLocalPlayerMoveFn         executeLocalPlayerMove;
	Inventory::IsItemTypeCodeKnownFn            isItemTypeCodeKnown;
	Inventory::ConfigurePlayerPageItemPolicyFn  configurePlayerPageItemPolicy;
	Inventory::ConfigurePlayerPageCharmPolicyFn configurePlayerPageCharmPolicy;
	Inventory::GetLocalPlayerFn                 getLocalPlayer;
	Inventory::GetCursorItemFn                  getCursorItem;
	Inventory::GetEquippedItemFn                getEquippedItem;
	Inventory::ForEachInventoryItemFn           forEachInventoryItem;
};

inline constexpr uint32_t InventoryServiceV1Version                     = 1;
inline constexpr uint32_t InventoryServiceV1Size                        = static_cast<uint32_t>(sizeof(InventoryServiceV1));
inline constexpr uint32_t InventoryServiceV1RequiredSize                = InventoryServiceV1Size;
inline constexpr uint32_t InventoryServiceV1ConfigurePlayerPageFieldEnd = static_cast<uint32_t>(offsetof(InventoryServiceV1, configurePlayerPage) + sizeof(Inventory::ConfigurePlayerPageFn));
inline constexpr uint32_t InventoryServiceV1GetPlayerPageConfigurationInfoFieldEnd
    = static_cast<uint32_t>(offsetof(InventoryServiceV1, getPlayerPageConfigurationInfo) + sizeof(Inventory::GetPlayerPageConfigurationInfoFn));
inline constexpr uint32_t InventoryServiceV1ExecuteLocalPlayerMoveFieldEnd = static_cast<uint32_t>(offsetof(InventoryServiceV1, executeLocalPlayerMove) + sizeof(Inventory::ExecuteLocalPlayerMoveFn));
inline constexpr uint32_t InventoryServiceV1IsItemTypeCodeKnownFieldEnd    = static_cast<uint32_t>(offsetof(InventoryServiceV1, isItemTypeCodeKnown) + sizeof(Inventory::IsItemTypeCodeKnownFn));
inline constexpr uint32_t InventoryServiceV1ConfigurePlayerPageItemPolicyFieldEnd
    = static_cast<uint32_t>(offsetof(InventoryServiceV1, configurePlayerPageItemPolicy) + sizeof(Inventory::ConfigurePlayerPageItemPolicyFn));
inline constexpr uint32_t InventoryServiceV1ConfigurePlayerPageCharmPolicyFieldEnd
    = static_cast<uint32_t>(offsetof(InventoryServiceV1, configurePlayerPageCharmPolicy) + sizeof(Inventory::ConfigurePlayerPageCharmPolicyFn));

inline auto HasInventoryServiceV1Field(const InventoryServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == InventoryServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<InventoryServiceV1>);
static_assert(std::is_trivially_copyable_v<InventoryServiceV1>);
static_assert(offsetof(InventoryServiceV1, registerPlayerPage) == 8);
static_assert(offsetof(InventoryServiceV1, unregisterPlayerPage) == 16);
static_assert(offsetof(InventoryServiceV1, getRegistrationInfo) == 24);
static_assert(offsetof(InventoryServiceV1, configurePlayerPage) == 32);
static_assert(offsetof(InventoryServiceV1, getPlayerPageConfigurationInfo) == 40);
static_assert(offsetof(InventoryServiceV1, executeLocalPlayerMove) == 48);
static_assert(offsetof(InventoryServiceV1, isItemTypeCodeKnown) == 56);
static_assert(offsetof(InventoryServiceV1, configurePlayerPageItemPolicy) == 64);
static_assert(offsetof(InventoryServiceV1, configurePlayerPageCharmPolicy) == 72);
static_assert(offsetof(InventoryServiceV1, getLocalPlayer) == 80);
static_assert(offsetof(InventoryServiceV1, getCursorItem) == 88);
static_assert(offsetof(InventoryServiceV1, getEquippedItem) == 96);
static_assert(offsetof(InventoryServiceV1, forEachInventoryItem) == 104);
static_assert(InventoryServiceV1RequiredSize == 112);
static_assert(InventoryServiceV1ConfigurePlayerPageFieldEnd == 40);
static_assert(InventoryServiceV1GetPlayerPageConfigurationInfoFieldEnd == 48);
static_assert(InventoryServiceV1ExecuteLocalPlayerMoveFieldEnd == 56);
static_assert(InventoryServiceV1IsItemTypeCodeKnownFieldEnd == 64);
static_assert(InventoryServiceV1ConfigurePlayerPageItemPolicyFieldEnd == 72);
static_assert(InventoryServiceV1ConfigurePlayerPageCharmPolicyFieldEnd == 80);
static_assert(sizeof(InventoryServiceV1) == 112);

}
