#pragma once

#include <D2RLPlugin/handles.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Items {

inline constexpr uint32_t MaxProperties         = 64;
inline constexpr uint32_t MaxTransactionInputs  = 64;
inline constexpr uint32_t MaxTransactionOutputs = 64;
inline constexpr uint32_t DefaultValue          = std::numeric_limits<uint32_t>::max();
inline constexpr uint32_t RandomQualityRecord   = std::numeric_limits<uint32_t>::max();

enum class Result : uint32_t {
	Success          = 0,
	InvalidArgument  = 1,
	Unsupported      = 2,
	Unavailable      = 3,
	Conflict         = 4,
	NotFound         = 5,
	Busy             = 6,
	OwnerInactive    = 7,
	OwnerMismatch    = 8,
	StaleHandle      = 9,
	CallbackFault    = 10,
	PolicyRejected   = 11,
	NotAuthoritative = 12,
};

enum class Quality : uint32_t {
	Unknown  = 0,
	Inferior = 1,
	Normal   = 2,
	Superior = 3,
	Magic    = 4,
	Set      = 5,
	Rare     = 6,
	Unique   = 7,
	Crafted  = 8,
};

enum class ItemContainer : uint32_t {
	Unknown       = 0,
	Equipment     = 1,
	Cursor        = 2,
	Belt          = 3,
	Inventory     = 4,
	Cube          = 5,
	Trade         = 6,
	PersonalStash = 7,
	SharedStash   = 8,
	CustomPage    = 9,
	Ground        = 10,
};

enum class Placement : uint32_t {
	Automatic = 0,
	Exact     = 1,
};

enum class SeedMode : uint32_t {
	Random        = 0,
	Deterministic = 1,
};

enum class SocketedItemPolicy : uint32_t {
	RejectIfNotEmpty = 0,
	DestroyContents  = 1,
};

enum class EditField : uint32_t {
	Quantity   = 1U << 0U,
	Durability = 1U << 1U,
	Identified = 1U << 2U,
	ItemLevel  = 1U << 3U,
	Sockets    = 1U << 4U,
	Ethereal   = 1U << 5U,
};

constexpr auto EditFieldBit(EditField field) noexcept -> uint32_t {
	return static_cast<uint32_t>(field);
}

inline constexpr uint32_t AllEditFields = EditFieldBit(EditField::Quantity) | EditFieldBit(EditField::Durability) | EditFieldBit(EditField::Identified) | EditFieldBit(EditField::ItemLevel)
                                        | EditFieldBit(EditField::Sockets) | EditFieldBit(EditField::Ethereal);

inline constexpr uint32_t ItemStateIdentified = 1U << 0U;
inline constexpr uint32_t ItemStateEthereal   = 1U << 1U;
inline constexpr uint32_t AllItemStates       = ItemStateIdentified | ItemStateEthereal;

constexpr auto ContainerBit(ItemContainer container) noexcept -> uint32_t {
	const uint32_t value = static_cast<uint32_t>(container);
	return value < 32 ? 1U << value : 0;
}

inline constexpr uint32_t AllItemContainers = 0xFFFFFFFFU;

// D2 item-table codes are four bytes and pad names shorter than four
// characters with ASCII spaces.
constexpr auto MakeItemCode(char first, char second = ' ', char third = ' ', char fourth = ' ') noexcept -> uint32_t {
	return static_cast<uint32_t>(static_cast<uint8_t>(first)) | (static_cast<uint32_t>(static_cast<uint8_t>(second)) << 8U) | (static_cast<uint32_t>(static_cast<uint8_t>(third)) << 16U)
	     | (static_cast<uint32_t>(static_cast<uint8_t>(fourth)) << 24U);
}

template <size_t Size>
constexpr auto MakeItemCode(const char (&value)[Size]) noexcept -> uint32_t {
	static_assert(Size >= 2 && Size <= 5, "Item code text must contain one through four characters.");
	return MakeItemCode(value[0], Size > 2 ? value[1] : ' ', Size > 3 ? value[2] : ' ', Size > 4 ? value[3] : ' ');
}

// A property means one Properties.txt row, not one ItemStatCost.txt stat.
// propertyId is that row's numeric *Id. parameter follows the Properties row
// and is zero for many ordinary properties. D2R applies it through the cube
// recipe engine, so one property may change several stats. Use equal minimum
// and maximum values for an exact result.
struct PropertySpec {
		uint32_t propertyId;
		int32_t  parameter;
		int32_t  minimum;
		int32_t  maximum;
};

constexpr auto MakeExactProperty(uint32_t propertyId, int32_t value, int32_t parameter = 0) noexcept -> PropertySpec {
	return { propertyId, parameter, value, value };
}

// Automatic finds the first free cell. Exact uses x/y as a top-left inventory
// cell or a ground coordinate. customPageHandle is required only for CustomPage.
struct ItemDestination {
		uint32_t      structSize;
		uint32_t      flags;
		ItemContainer container;
		Placement     placement;
		uint64_t      customPageHandle;
		uint32_t      x;
		uint32_t      y;
};

// qualityRecordId is a zero-based SetItems or UniqueItems row for forced Set or
// Unique creation. RandomQualityRecord lets D2R select a compatible row.
// quantity and durability use DefaultValue to keep the native default.
// socketCount is the exact number of empty sockets requested.
struct ItemCreateSpec {
		uint32_t            structSize;
		uint32_t            flags;
		uint32_t            code;
		Quality             quality;
		uint32_t            qualityRecordId;
		uint32_t            itemLevel;
		uint32_t            prefixIds[3];
		uint32_t            suffixIds[3];
		SeedMode            seedMode;
		uint32_t            generationSeed;
		uint32_t            itemSeed;
		uint32_t            quantity;
		uint32_t            durability;
		uint32_t            socketCount;
		uint32_t            stateFlags;
		uint32_t            propertyCount;
		const PropertySpec* properties;
		ItemDestination     destination;
};

// This is a copy. quantity is one for a non-stackable item. qualityRecordId is
// a zero-based SetItems/UniqueItems row, or -1 when there is no such row.
// prefixes and suffixes are native one-based MagicAffix ids. sharedStashPage is
// UINT32_MAX when the tab is not known.
struct ItemInfo {
		uint32_t      structSize;
		uint32_t      flags;
		ItemHandle    handle;
		uint32_t      code;
		uint32_t      classId;
		uint32_t      runtimeId;
		ItemContainer container;
		int32_t       inventoryPage;
		uint32_t      sharedStashPage;
		int32_t       bodyLocation;
		int32_t       x;
		int32_t       y;
		Quality       quality;
		uint32_t      itemLevel;
		int32_t       quantity;
		int32_t       durability;
		int32_t       maximumDurability;
		int32_t       qualityRecordId;
		uint32_t      generationSeed;
		uint32_t      itemSeed;
		uint32_t      stateFlags;
		uint32_t      socketCount;
		uint32_t      socketedItemCount;
		uint32_t      prefixIds[3];
		uint32_t      suffixIds[3];
};

// fields chooses what changes. The public handle and native item stay intact.
// Turning ethereal off is unsupported because D2R's native 3/2 base-stat
// conversion cannot be safely reversed.
struct ItemEdit {
		uint32_t structSize;
		uint32_t flags;
		uint32_t fields;
		uint32_t stateFlags;
		uint32_t quantity;
		uint32_t durability;
		uint32_t itemLevel;
		uint32_t socketCount;
};

// quantity is the logical amount consumed. Non-stackable items have a logical
// quantity of one. Consuming less than a stack updates it in place; consuming
// the whole logical quantity destroys it after the transaction commits.
struct TransactionInput {
		ItemHandle         item;
		uint32_t           quantity;
		SocketedItemPolicy socketedItemPolicy;
};

// Every input and output belongs to player. D2RLoader writes output handles only
// after the whole exchange commits. outputCapacity must cover outputCount. If
// validation, creation, placement, or capacity fails, nothing commits and every
// staged native item is rolled back.
struct Transaction {
		uint32_t                structSize;
		uint32_t                flags;
		PlayerHandle            player;
		uint32_t                inputCount;
		uint32_t                outputCount;
		const TransactionInput* inputs;
		const ItemCreateSpec*   outputs;
		ItemHandle*             outputItems;
		uint32_t                outputCapacity;
		uint32_t                reserved;
};

struct TransactionResult {
		uint32_t structSize;
		uint32_t flags;
		uint32_t outputCount;
		uint32_t reserved;
};

using NativeItemEditCallback = void(__cdecl*)(const PluginContext* context, void* nativeItem, void* userData) noexcept;

inline constexpr uint32_t PropertySpecSize              = static_cast<uint32_t>(sizeof(PropertySpec));
inline constexpr uint32_t ItemDestinationSize           = static_cast<uint32_t>(sizeof(ItemDestination));
inline constexpr uint32_t ItemDestinationRequiredSize   = ItemDestinationSize;
inline constexpr uint32_t ItemCreateSpecSize            = static_cast<uint32_t>(sizeof(ItemCreateSpec));
inline constexpr uint32_t ItemCreateSpecRequiredSize    = ItemCreateSpecSize;
inline constexpr uint32_t ItemInfoSize                  = static_cast<uint32_t>(sizeof(ItemInfo));
inline constexpr uint32_t ItemInfoRequiredSize          = ItemInfoSize;
inline constexpr uint32_t ItemEditSize                  = static_cast<uint32_t>(sizeof(ItemEdit));
inline constexpr uint32_t ItemEditRequiredSize          = ItemEditSize;
inline constexpr uint32_t TransactionSize               = static_cast<uint32_t>(sizeof(Transaction));
inline constexpr uint32_t TransactionRequiredSize       = TransactionSize;
inline constexpr uint32_t TransactionResultSize         = static_cast<uint32_t>(sizeof(TransactionResult));
inline constexpr uint32_t TransactionResultRequiredSize = TransactionResultSize;

using GetItemInfoFn        = Result(__cdecl*)(const PluginContext* context, ItemHandle item, ItemInfo* info) noexcept;
// Mutations require the authoritative game thread. Queue them with
// ThreadServiceV1::runOnGameThread. Local games and TCP/IP hosts are
// authoritative; remote clients receive NotAuthoritative. Normal operations do
// not require PluginFlags::NativeHooks.
using CreateItemFn         = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, const ItemCreateSpec* spec, ItemHandle* item) noexcept;
using EditItemFn           = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, ItemHandle item, const ItemEdit* edit) noexcept;
using DestroyItemFn        = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, ItemHandle item, SocketedItemPolicy socketedItemPolicy) noexcept;
using ExecuteTransactionFn = Result(__cdecl*)(const PluginContext* context, const Transaction* transaction, TransactionResult* result) noexcept;
// Raw-pointer escape hatch. This requires PluginFlags::NativeHooks and the game
// thread. The pointer expires when the synchronous callback returns. D2RLoader
// does not validate or publish changes made through it.
using EditNativeItemFn     = Result(__cdecl*)(const PluginContext* context, ItemHandle item, NativeItemEditCallback callback, void* userData) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(Quality) == sizeof(uint32_t));
static_assert(sizeof(ItemContainer) == sizeof(uint32_t));
static_assert(sizeof(Placement) == sizeof(uint32_t));
static_assert(sizeof(SeedMode) == sizeof(uint32_t));
static_assert(sizeof(SocketedItemPolicy) == sizeof(uint32_t));
static_assert(sizeof(EditField) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<PropertySpec> && std::is_trivially_copyable_v<PropertySpec>);
static_assert(std::is_standard_layout_v<ItemDestination> && std::is_trivially_copyable_v<ItemDestination>);
static_assert(std::is_standard_layout_v<ItemCreateSpec> && std::is_trivially_copyable_v<ItemCreateSpec>);
static_assert(std::is_standard_layout_v<ItemInfo> && std::is_trivially_copyable_v<ItemInfo>);
static_assert(std::is_standard_layout_v<ItemEdit> && std::is_trivially_copyable_v<ItemEdit>);
static_assert(std::is_standard_layout_v<TransactionInput> && std::is_trivially_copyable_v<TransactionInput>);
static_assert(std::is_standard_layout_v<Transaction> && std::is_trivially_copyable_v<Transaction>);
static_assert(std::is_standard_layout_v<TransactionResult> && std::is_trivially_copyable_v<TransactionResult>);
static_assert(sizeof(PropertySpec) == 16);
static_assert(sizeof(ItemDestination) == 32);
static_assert(sizeof(ItemCreateSpec) == 120);
static_assert(sizeof(ItemInfo) == 120);
static_assert(sizeof(ItemEdit) == 32);
static_assert(sizeof(TransactionInput) == 16);
static_assert(offsetof(Transaction, inputs) == 24);
static_assert(offsetof(Transaction, outputItems) == 40);
static_assert(TransactionRequiredSize == 56);
static_assert(sizeof(Transaction) == 56);
static_assert(TransactionResultRequiredSize == 16);
static_assert(sizeof(TransactionResult) == 16);
static_assert(MakeItemCode("r01") == MakeItemCode('r', '0', '1', ' '));

}

struct ItemServiceV1 {
		uint32_t                    serviceSize;
		uint32_t                    serviceVersion;
		Items::GetItemInfoFn        getItemInfo;
		Items::CreateItemFn         createItem;
		Items::EditItemFn           editItem;
		Items::DestroyItemFn        destroyItem;
		Items::ExecuteTransactionFn executeTransaction;
		Items::EditNativeItemFn     editNativeItem;
};

inline constexpr uint32_t ItemServiceV1Version      = 1;
inline constexpr uint32_t ItemServiceV1Size         = static_cast<uint32_t>(sizeof(ItemServiceV1));
inline constexpr uint32_t ItemServiceV1RequiredSize = ItemServiceV1Size;

inline auto HasItemServiceV1Field(const ItemServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ItemServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ItemServiceV1>);
static_assert(std::is_trivially_copyable_v<ItemServiceV1>);
static_assert(offsetof(ItemServiceV1, getItemInfo) == 8);
static_assert(offsetof(ItemServiceV1, createItem) == 16);
static_assert(offsetof(ItemServiceV1, editItem) == 24);
static_assert(offsetof(ItemServiceV1, destroyItem) == 32);
static_assert(offsetof(ItemServiceV1, executeTransaction) == 40);
static_assert(offsetof(ItemServiceV1, editNativeItem) == 48);
static_assert(ItemServiceV1RequiredSize == 56);
static_assert(sizeof(ItemServiceV1) == 56);

}
