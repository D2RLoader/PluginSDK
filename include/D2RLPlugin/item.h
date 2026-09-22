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

inline constexpr uint32_t MaxProperties             = 64;
inline constexpr uint32_t MaxTransactionInputs      = 64;
inline constexpr uint32_t MaxTransactionOutputs     = 64;
inline constexpr uint32_t MaxExistingItemOperations = 4'096;
inline constexpr uint32_t DefaultValue              = std::numeric_limits<uint32_t>::max();
inline constexpr uint32_t RandomQualityRecord       = std::numeric_limits<uint32_t>::max();
inline constexpr uint32_t NoFailedOperation         = std::numeric_limits<uint32_t>::max();

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

enum class ItemServiceCapability : uint64_t {
	SharedStashWrite = 1ULL << 0U,
	AffixAugment     = 1ULL << 1U,
};

constexpr auto ItemServiceCapabilityBit(ItemServiceCapability capability) noexcept -> uint64_t {
	return static_cast<uint64_t>(capability);
}

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

enum class ItemCreateFlag : uint32_t {
	AllowDuplicateUnique = 1U << 0U,
};

constexpr auto ItemCreateFlagBit(ItemCreateFlag flag) noexcept -> uint32_t {
	return static_cast<uint32_t>(flag);
}

inline constexpr uint32_t AllItemCreateFlags = ItemCreateFlagBit(ItemCreateFlag::AllowDuplicateUnique);

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
// sharedStashPage is a zero-based normal tab number. It is used only when the
// destination is SharedStash and structSize includes that field.
struct ItemDestination {
	uint32_t      structSize;
	uint32_t      flags;
	ItemContainer container;
	Placement     placement;
	uint64_t      customPageHandle;
	uint32_t      x;
	uint32_t      y;
	uint32_t      sharedStashPage;
	uint32_t      reserved;
};

// qualityRecordId is a zero-based SetItems or UniqueItems row for forced Set or
// Unique creation. RandomQualityRecord lets D2R select a compatible row. D2R's
// once-per-game rule for each Unique row remains active by default. Set the
// AllowDuplicateUnique flag only when the plugin intentionally permits a
// previously generated Unique row to be selected again.
// prefixIds and suffixIds are one-based IDs in D2R's combined MagicSuffix,
// MagicPrefix, and AutoMagic table. Zero leaves that slot unforced. A prefix ID
// must point to MagicPrefix, and a suffix ID must point to MagicSuffix. For
// example, with 10 suffix rows, MagicPrefix table-local ID 1 has combined ID 11.
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
// prefixIds and suffixIds use the same combined IDs as ItemCreateSpec.
// sharedStashPage is UINT32_MAX when the tab is not known.
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

enum class ExistingItemOperationKind : uint32_t {
	Debit = 1,
	Edit  = 2,
	Move  = 3,
};

struct ExistingItemDebit {
	uint32_t quantity;
	uint32_t reserved;
};

// Only reversible fields are accepted here. Quantity uses Debit instead.
struct ExistingItemEdit {
	uint32_t fields;
	uint32_t stateFlags;
	uint32_t durability;
	uint32_t itemLevel;
};

struct ExistingItemMove {
	ItemDestination destination;
};

inline constexpr uint32_t AllExistingItemEditFields = EditFieldBit(EditField::Durability) | EditFieldBit(EditField::Identified) | EditFieldBit(EditField::ItemLevel);

// Each item handle may appear only once. Debit must leave the item with a
// positive logical quantity, so every operation preserves its handle and native
// identity. Debit and Edit accept stored or cursor items. Move accepts stored
// inventory, Cube, personal stash, shared stash, and the current custom page.
// Equipment, cursor, belt, trade, corpse, and ground moves are rejected before
// mutation. Shared-stash moves require ItemServiceCapability::SharedStashWrite.
struct ExistingItemOperation {
	uint32_t                  structSize;
	uint32_t                  flags;
	ExistingItemOperationKind kind;
	uint32_t                  reserved;
	ItemHandle                item;

	union {
		ExistingItemDebit debit;
		ExistingItemEdit  edit;
		ExistingItemMove  move;
		uint8_t           reservedData[32];
	};
};

struct ExistingItemTransaction {
	uint32_t                     structSize;
	uint32_t                     flags;
	PlayerHandle                 player;
	uint32_t                     operationCount;
	uint32_t                     reserved;
	const ExistingItemOperation* operations;
};

// failureIndex is NoFailedOperation on success or when failure happened before
// an operation could be selected. committedOperationCount is zero on failure.
struct ExistingItemTransactionResult {
	uint32_t structSize;
	uint32_t flags;
	uint32_t committedOperationCount;
	uint32_t failureIndex;
};

// Splits quantity from one stored stack and puts the new stack on the cursor.
// The source keeps its handle and saved identity. The cursor must be empty,
// and quantity must leave at least one item in the source stack.
struct SplitStackRequest {
	uint32_t     structSize;
	uint32_t     flags;
	PlayerHandle player;
	ItemHandle   sourceItem;
	uint32_t     quantity;
	uint32_t     reserved;
};

// cursorItem is the new stack. remainingQuantity is the source stack's new
// quantity. Both values are cleared when the split fails.
struct SplitStackResult {
	uint32_t   structSize;
	uint32_t   flags;
	ItemHandle cursorItem;
	uint32_t   remainingQuantity;
	uint32_t   reserved;
};

enum class AffixSelection : uint32_t {
	RandomEligible = 0,
	ExplicitId     = 1,
};

enum class AffixKind : uint32_t {
	Either = 0,
	Prefix = 1,
	Suffix = 2,
};

enum class AffixAugmentFailure : uint32_t {
	None              = 0,
	InvalidRequest    = 1,
	InvalidItem       = 2,
	AtCapacity        = 3,
	SideAtCapacity    = 4,
	UnknownAffix      = 5,
	IneligibleAffix   = 6,
	NoEligibleAffix   = 7,
	NativeApplyFailed = 8,
	InvariantMismatch = 9,
	PaymentFailed     = 10,
	RollbackFailed    = 11,
};

// Adds one eligible prefix or suffix to an existing Magic or Rare item. RandomEligible
// requires affixId 0 and accepts Either, Prefix, or Suffix. ExplicitId requires
// a combined, one-based affix ID and an exact side. These IDs match ItemInfo.
// A zero maximum uses the native limit. A smaller maximum lets a plugin apply
// its own crafting rule. Magic items allow one prefix and one suffix. Rare
// jewels have a native total limit of four; other Rare items have a limit of six.
//
// Leave paymentItem invalid and paymentQuantity zero for a free operation.
// Otherwise, the payment must be a different item owned by the same player.
// The target and payment must be in an inventory, Cube, personal stash, current
// custom page, or on the cursor. Shared-stash items are not supported. The
// service can consume part of a stack or the whole payment item.
struct AffixAugmentRequest {
	uint32_t       structSize;
	uint32_t       flags;
	PlayerHandle   player;
	ItemHandle     item;
	ItemHandle     paymentItem;
	uint32_t       paymentQuantity;
	AffixSelection selection;
	AffixKind      kind;
	uint32_t       affixId;
	uint32_t       maxPrefixes;
	uint32_t       maxSuffixes;
	uint32_t       maxAffixes;
	uint32_t       reserved;
};

// On success, appliedSlot is 0..2 and paymentConsumed is the amount removed.
// On an ordinary failure, both items are unchanged. RollbackFailed means an
// unexpected native error prevented that guarantee. D2RLoader then rejects
// more affix changes until the next game session.
struct AffixAugmentResult {
	uint32_t            structSize;
	uint32_t            flags;
	AffixAugmentFailure failure;
	AffixKind           appliedKind;
	uint32_t            appliedAffixId;
	uint32_t            appliedSlot;
	uint32_t            affixesBefore;
	uint32_t            affixesAfter;
	uint32_t            paymentConsumed;
	uint32_t            reserved;
};

using NativeItemEditCallback = void(__cdecl*)(const PluginContext* context, void* nativeItem, void* userData) noexcept;

inline constexpr uint32_t PropertySpecSize                             = static_cast<uint32_t>(sizeof(PropertySpec));
inline constexpr uint32_t ItemDestinationSize                          = static_cast<uint32_t>(sizeof(ItemDestination));
inline constexpr uint32_t ItemDestinationRequiredSize                  = static_cast<uint32_t>(offsetof(ItemDestination, sharedStashPage));
inline constexpr uint32_t ItemDestinationSharedStashPageFieldEnd       = static_cast<uint32_t>(offsetof(ItemDestination, sharedStashPage) + sizeof(uint32_t));
inline constexpr uint32_t ItemCreateSpecSize                           = static_cast<uint32_t>(sizeof(ItemCreateSpec));
inline constexpr uint32_t ItemCreateSpecRequiredSize                   = static_cast<uint32_t>(offsetof(ItemCreateSpec, destination) + ItemDestinationRequiredSize);
inline constexpr uint32_t ItemCreateSpecSharedStashPageFieldEnd        = static_cast<uint32_t>(offsetof(ItemCreateSpec, destination) + ItemDestinationSharedStashPageFieldEnd);
inline constexpr uint32_t ItemInfoSize                                 = static_cast<uint32_t>(sizeof(ItemInfo));
inline constexpr uint32_t ItemInfoRequiredSize                         = ItemInfoSize;
inline constexpr uint32_t ItemEditSize                                 = static_cast<uint32_t>(sizeof(ItemEdit));
inline constexpr uint32_t ItemEditRequiredSize                         = ItemEditSize;
inline constexpr uint32_t TransactionSize                              = static_cast<uint32_t>(sizeof(Transaction));
inline constexpr uint32_t TransactionRequiredSize                      = TransactionSize;
inline constexpr uint32_t TransactionResultSize                        = static_cast<uint32_t>(sizeof(TransactionResult));
inline constexpr uint32_t TransactionResultRequiredSize                = TransactionResultSize;
inline constexpr uint32_t ExistingItemOperationSize                    = static_cast<uint32_t>(sizeof(ExistingItemOperation));
inline constexpr uint32_t ExistingItemOperationRequiredSize            = static_cast<uint32_t>(offsetof(ExistingItemOperation, move) + ItemDestinationRequiredSize);
inline constexpr uint32_t ExistingItemOperationSharedStashPageFieldEnd = static_cast<uint32_t>(offsetof(ExistingItemOperation, move) + ItemDestinationSharedStashPageFieldEnd);
inline constexpr uint32_t ExistingItemTransactionSize                  = static_cast<uint32_t>(sizeof(ExistingItemTransaction));
inline constexpr uint32_t ExistingItemTransactionRequiredSize          = ExistingItemTransactionSize;
inline constexpr uint32_t ExistingItemTransactionResultSize            = static_cast<uint32_t>(sizeof(ExistingItemTransactionResult));
inline constexpr uint32_t ExistingItemTransactionResultRequiredSize    = ExistingItemTransactionResultSize;
inline constexpr uint32_t SplitStackRequestSize                        = static_cast<uint32_t>(sizeof(SplitStackRequest));
inline constexpr uint32_t SplitStackRequestRequiredSize                = SplitStackRequestSize;
inline constexpr uint32_t SplitStackResultSize                         = static_cast<uint32_t>(sizeof(SplitStackResult));
inline constexpr uint32_t SplitStackResultRequiredSize                 = SplitStackResultSize;
inline constexpr uint32_t AffixAugmentRequestSize                      = static_cast<uint32_t>(sizeof(AffixAugmentRequest));
inline constexpr uint32_t AffixAugmentRequestRequiredSize              = AffixAugmentRequestSize;
inline constexpr uint32_t AffixAugmentResultSize                       = static_cast<uint32_t>(sizeof(AffixAugmentResult));
inline constexpr uint32_t AffixAugmentResultRequiredSize               = AffixAugmentResultSize;

using GetItemInfoFn                    = Result(__cdecl*)(const PluginContext* context, ItemHandle item, ItemInfo* info) noexcept;
// Mutations require the authoritative game thread. Queue them with
// ThreadService::runOnGameThread. Local games and TCP/IP hosts are
// authoritative; remote clients receive NotAuthoritative. Normal operations do
// not require PluginFlags::NativeHooks.
using CreateItemFn                     = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, const ItemCreateSpec* spec, ItemHandle* item) noexcept;
using EditItemFn                       = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, ItemHandle item, const ItemEdit* edit) noexcept;
using DestroyItemFn                    = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, ItemHandle item, SocketedItemPolicy socketedItemPolicy) noexcept;
using ExecuteTransactionFn             = Result(__cdecl*)(const PluginContext* context, const Transaction* transaction, TransactionResult* result) noexcept;
using ExecuteExistingItemTransactionFn = Result(__cdecl*)(const PluginContext* context, const ExistingItemTransaction* transaction, ExistingItemTransactionResult* result) noexcept;
using SplitStackFn                     = Result(__cdecl*)(const PluginContext* context, const SplitStackRequest* request, SplitStackResult* result) noexcept;
using AugmentItemAffixFn               = Result(__cdecl*)(const PluginContext* context, const AffixAugmentRequest* request, AffixAugmentResult* result) noexcept;
// Raw-pointer escape hatch. This requires PluginFlags::NativeHooks and the game
// thread. The pointer expires when the synchronous callback returns. D2RLoader
// does not validate or publish changes made through it.
using EditNativeItemFn                 = Result(__cdecl*)(const PluginContext* context, ItemHandle item, NativeItemEditCallback callback, void* userData) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(ItemServiceCapability) == sizeof(uint64_t));
static_assert(sizeof(Quality) == sizeof(uint32_t));
static_assert(sizeof(ItemContainer) == sizeof(uint32_t));
static_assert(sizeof(Placement) == sizeof(uint32_t));
static_assert(sizeof(SeedMode) == sizeof(uint32_t));
static_assert(sizeof(ItemCreateFlag) == sizeof(uint32_t));
static_assert(sizeof(SocketedItemPolicy) == sizeof(uint32_t));
static_assert(sizeof(EditField) == sizeof(uint32_t));
static_assert(sizeof(ExistingItemOperationKind) == sizeof(uint32_t));
static_assert(sizeof(AffixSelection) == sizeof(uint32_t));
static_assert(sizeof(AffixKind) == sizeof(uint32_t));
static_assert(sizeof(AffixAugmentFailure) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<PropertySpec> && std::is_trivially_copyable_v<PropertySpec>);
static_assert(std::is_standard_layout_v<ItemDestination> && std::is_trivially_copyable_v<ItemDestination>);
static_assert(std::is_standard_layout_v<ItemCreateSpec> && std::is_trivially_copyable_v<ItemCreateSpec>);
static_assert(std::is_standard_layout_v<ItemInfo> && std::is_trivially_copyable_v<ItemInfo>);
static_assert(std::is_standard_layout_v<ItemEdit> && std::is_trivially_copyable_v<ItemEdit>);
static_assert(std::is_standard_layout_v<TransactionInput> && std::is_trivially_copyable_v<TransactionInput>);
static_assert(std::is_standard_layout_v<Transaction> && std::is_trivially_copyable_v<Transaction>);
static_assert(std::is_standard_layout_v<TransactionResult> && std::is_trivially_copyable_v<TransactionResult>);
static_assert(std::is_standard_layout_v<ExistingItemDebit> && std::is_trivially_copyable_v<ExistingItemDebit>);
static_assert(std::is_standard_layout_v<ExistingItemEdit> && std::is_trivially_copyable_v<ExistingItemEdit>);
static_assert(std::is_standard_layout_v<ExistingItemMove> && std::is_trivially_copyable_v<ExistingItemMove>);
static_assert(std::is_standard_layout_v<ExistingItemOperation> && std::is_trivially_copyable_v<ExistingItemOperation>);
static_assert(std::is_standard_layout_v<ExistingItemTransaction> && std::is_trivially_copyable_v<ExistingItemTransaction>);
static_assert(std::is_standard_layout_v<ExistingItemTransactionResult> && std::is_trivially_copyable_v<ExistingItemTransactionResult>);
static_assert(std::is_standard_layout_v<SplitStackRequest> && std::is_trivially_copyable_v<SplitStackRequest>);
static_assert(std::is_standard_layout_v<SplitStackResult> && std::is_trivially_copyable_v<SplitStackResult>);
static_assert(std::is_standard_layout_v<AffixAugmentRequest> && std::is_trivially_copyable_v<AffixAugmentRequest>);
static_assert(std::is_standard_layout_v<AffixAugmentResult> && std::is_trivially_copyable_v<AffixAugmentResult>);
static_assert(sizeof(PropertySpec) == 16);
static_assert(ItemDestinationRequiredSize == 32);
static_assert(ItemDestinationSharedStashPageFieldEnd == 36);
static_assert(sizeof(ItemDestination) == 40);
static_assert(ItemCreateSpecRequiredSize == 120);
static_assert(ItemCreateSpecSharedStashPageFieldEnd == 124);
static_assert(sizeof(ItemCreateSpec) == 128);
static_assert(sizeof(ItemInfo) == 120);
static_assert(sizeof(ItemEdit) == 32);
static_assert(sizeof(TransactionInput) == 16);
static_assert(offsetof(Transaction, inputs) == 24);
static_assert(offsetof(Transaction, outputItems) == 40);
static_assert(TransactionRequiredSize == 56);
static_assert(sizeof(Transaction) == 56);
static_assert(TransactionResultRequiredSize == 16);
static_assert(sizeof(TransactionResult) == 16);
static_assert(sizeof(ExistingItemDebit) == 8);
static_assert(sizeof(ExistingItemEdit) == 16);
static_assert(sizeof(ExistingItemMove) == 40);
static_assert(offsetof(ExistingItemOperation, item) == 16);
static_assert(ExistingItemOperationRequiredSize == 56);
static_assert(ExistingItemOperationSharedStashPageFieldEnd == 60);
static_assert(sizeof(ExistingItemOperation) == 64);
static_assert(offsetof(ExistingItemTransaction, operations) == 24);
static_assert(ExistingItemTransactionRequiredSize == 32);
static_assert(sizeof(ExistingItemTransaction) == 32);
static_assert(ExistingItemTransactionResultRequiredSize == 16);
static_assert(sizeof(ExistingItemTransactionResult) == 16);
static_assert(offsetof(SplitStackRequest, player) == 8);
static_assert(offsetof(SplitStackRequest, sourceItem) == 16);
static_assert(SplitStackRequestRequiredSize == 32);
static_assert(sizeof(SplitStackRequest) == 32);
static_assert(offsetof(SplitStackResult, cursorItem) == 8);
static_assert(SplitStackResultRequiredSize == 24);
static_assert(sizeof(SplitStackResult) == 24);
static_assert(sizeof(AffixAugmentRequest) == 64);
static_assert(sizeof(AffixAugmentResult) == 40);
static_assert(MakeItemCode("r01") == MakeItemCode('r', '0', '1', ' '));

}

struct ItemService {
	static constexpr ServiceId Id         = ServiceId::Item;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                                serviceSize;
	uint32_t                                serviceVersion;
	Items::GetItemInfoFn                    getItemInfo;
	Items::CreateItemFn                     createItem;
	Items::EditItemFn                       editItem;
	Items::DestroyItemFn                    destroyItem;
	Items::ExecuteTransactionFn             executeTransaction;
	Items::EditNativeItemFn                 editNativeItem;
	Items::ExecuteExistingItemTransactionFn executeExistingItemTransaction;
	Items::SplitStackFn                     splitStack;
	// This field is optional. HasItemServiceCapability checks its size and bit.
	uint64_t                                capabilities;
	Items::AugmentItemAffixFn               augmentItemAffix;
};

inline constexpr uint32_t ItemServiceSize                 = static_cast<uint32_t>(sizeof(ItemService));
inline constexpr uint32_t ItemServiceRequiredSize         = static_cast<uint32_t>(offsetof(ItemService, executeExistingItemTransaction) + sizeof(Items::ExecuteExistingItemTransactionFn));
inline constexpr uint32_t ItemServiceSplitStackFieldEnd   = static_cast<uint32_t>(offsetof(ItemService, splitStack) + sizeof(Items::SplitStackFn));
inline constexpr uint32_t ItemServiceCapabilitiesFieldEnd = static_cast<uint32_t>(offsetof(ItemService, capabilities) + sizeof(uint64_t));
inline constexpr uint32_t ItemServiceAffixAugmentFieldEnd = static_cast<uint32_t>(offsetof(ItemService, augmentItemAffix) + sizeof(Items::AugmentItemAffixFn));

inline constexpr auto HasItemServiceField(const ItemService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ItemService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

inline constexpr auto HasItemServiceCapability(const ItemService* service, Items::ItemServiceCapability capability) noexcept -> bool {
	return HasItemServiceField(service, ItemServiceCapabilitiesFieldEnd) && (service->capabilities & Items::ItemServiceCapabilityBit(capability)) != 0;
}

static_assert(std::is_standard_layout_v<ItemService>);
static_assert(std::is_trivially_copyable_v<ItemService>);
static_assert(offsetof(ItemService, getItemInfo) == 8);
static_assert(offsetof(ItemService, createItem) == 16);
static_assert(offsetof(ItemService, editItem) == 24);
static_assert(offsetof(ItemService, destroyItem) == 32);
static_assert(offsetof(ItemService, executeTransaction) == 40);
static_assert(offsetof(ItemService, editNativeItem) == 48);
static_assert(offsetof(ItemService, executeExistingItemTransaction) == 56);
static_assert(offsetof(ItemService, splitStack) == 64);
static_assert(offsetof(ItemService, capabilities) == 72);
static_assert(offsetof(ItemService, augmentItemAffix) == 80);
static_assert(ItemServiceRequiredSize == 64);
static_assert(ItemServiceSplitStackFieldEnd == 72);
static_assert(ItemServiceCapabilitiesFieldEnd == 80);
static_assert(ItemServiceAffixAugmentFieldEnd == 88);
static_assert(sizeof(ItemService) == 88);

}
