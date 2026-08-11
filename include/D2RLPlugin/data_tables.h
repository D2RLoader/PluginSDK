#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace DataTables {

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

enum class Bank : uint32_t {
	Classic = 1,
	Lod     = 2,
	Rotw    = 3,
};

// These ids are permanent ABI values. Later API versions may add ids, but do not
// renumber the existing ones.
enum class TableId : uint32_t {
	Unknown              = 0,
	PlayerClass          = 1,
	BodyLocs             = 2,
	StorePage            = 3,
	ElemTypes            = 4,
	HitClass             = 5,
	MonMode              = 6, // full compiled rows
	PlrMode              = 7,
	ActCode              = 8,
	ActInfo              = 9,
	SkillCalc            = 10,
	MissCalc             = 11,
	Skills               = 12, // full compiled rows
	Events               = 13,
	CompCode             = 14,
	MonAi                = 15,
	Properties           = 16,
	PropertyGroups       = 17,
	States               = 18,
	Hireling             = 19,
	Npc                  = 20,
	Colors               = 21,
	MonStats             = 22,
	MonSounds            = 23,
	MonStats2            = 24,
	MonPlace             = 25,
	MonPreset            = 26,
	WanderingMon         = 27,
	SuperUniques         = 28,
	Missiles             = 29,
	MonLvl               = 30,
	MonSeq               = 31,
	SkillDesc            = 32,
	Overlay              = 33,
	CharStats            = 34,
	ItemStatCost         = 35,
	MonEquip             = 36,
	MonPet               = 37,
	PetType              = 38,
	ItemUiCategories     = 39,
	RunewordUiCategories = 40,
	ItemTypes            = 41,
	Sets                 = 42,
	SetItems             = 43,
	UniqueItems          = 44,
	MonProp              = 45,
	MonType              = 46,
	MonUMod              = 47,
	Levels               = 48,
	LevelGroups          = 49,
	LevelDefs            = 50,
	LvlPrest             = 51,
	Experience           = 52,
	DifficultyLevels     = 53,
	Objects              = 54,
	ObjGroup             = 55,
	Belts                = 56,
	CubeMain             = 57,
	Inventory            = 58,
	Items                = 59, // combined Weapons, Armor, and Misc rows
	MagicAffixes         = 60, // combined MagicSuffix, MagicPrefix, and AutoMagic rows
	QualityItems         = 61,
	Gems                 = 62,
	Books                = 63,
	LowQualityItems      = 64,
	ItemRatio            = 65,
	Runes                = 66,
	LvlTypes             = 67,
	LvlWarp              = 68,
	LvlMaze              = 69,
	UniquePrefix         = 70,
	UniqueSuffix         = 71,
	UniqueAppellation    = 72,
	Shrines              = 73,
	Composit             = 74,
	ArmType              = 75,
};

// Set structSize to TableViewSize before calling getTable. rows points to the
// active compiled data. Check rowSize before casting. The pointer is read-only,
// game-thread-only, and expires when the next table load starts. revision
// matches DataTablesLoadedEvent.
struct TableView {
	uint32_t    structSize;
	uint32_t    flags;
	TableId     tableId;
	Bank        bank;
	uint64_t    revision;
	const void* rows;
	uint32_t    rowCount;
	uint32_t    rowSize;
	uint64_t    reserved;
};

// Set structSize to RowViewSize before a lookup. row follows the same ownership,
// lifetime, and threading rules as TableView::rows.
struct RowView {
	uint32_t    structSize;
	uint32_t    flags;
	TableId     tableId;
	Bank        bank;
	uint64_t    revision;
	const void* row;
	uint32_t    rowIndex;
	uint32_t    rowSize;
	uint64_t    reserved;
};

inline constexpr uint32_t TableViewSize         = static_cast<uint32_t>(sizeof(TableView));
inline constexpr uint32_t TableViewRequiredSize = static_cast<uint32_t>(offsetof(TableView, reserved));
inline constexpr uint32_t RowViewSize           = static_cast<uint32_t>(sizeof(RowView));
inline constexpr uint32_t RowViewRequiredSize   = static_cast<uint32_t>(offsetof(RowView, reserved));

inline auto HasTableViewField(const TableView* view, uint32_t fieldEndOffset) noexcept -> bool {
	return view != nullptr && view->structSize >= fieldEndOffset;
}

inline auto HasRowViewField(const RowView* view, uint32_t fieldEndOffset) noexcept -> bool {
	return view != nullptr && view->structSize >= fieldEndOffset;
}

template <typename Row>
inline auto Rows(const TableView* view) noexcept -> std::span<const Row> {
	if (!HasTableViewField(view, TableViewRequiredSize)) {
		return {};
	}

	if (view->rowSize != sizeof(Row)) {
		return {};
	}

	if (view->rowCount != 0 && view->rows == nullptr) {
		return {};
	}

	const auto* rows = static_cast<const Row*>(view->rows);
	return { rows, view->rowCount };
}

template <typename Row>
inline auto AsRow(const RowView* view) noexcept -> const Row* {
	return HasRowViewField(view, RowViewRequiredSize) && view->rowSize == sizeof(Row) ? static_cast<const Row*>(view->row) : nullptr;
}

constexpr auto MakeFourCC(char a, char b = '\0', char c = '\0', char d = '\0') noexcept -> uint32_t {
	return static_cast<uint32_t>(static_cast<uint8_t>(a)) | (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8U) | (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16U)
	     | (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24U);
}

using GetTableFn      = Result(__cdecl*)(const PluginContext* context, Bank bank, TableId tableId, TableView* view) noexcept;
using GetRowFn        = Result(__cdecl*)(const PluginContext* context, Bank bank, TableId tableId, uint32_t rowIndex, RowView* view) noexcept;
using FindRowByIdFn   = Result(__cdecl*)(const PluginContext* context, Bank bank, TableId tableId, uint32_t id, RowView* view) noexcept;
using FindRowByCodeFn = Result(__cdecl*)(const PluginContext* context, Bank bank, TableId tableId, uint32_t code, RowView* view) noexcept;

// getTable and getRow support every listed table. findRowById supports Items,
// ItemTypes, Skills, and Levels. findRowByCode supports Items and ItemTypes.
// Calls made off the captured game thread return Busy.

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(Bank) == sizeof(uint32_t));
static_assert(sizeof(TableId) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<TableView>);
static_assert(std::is_trivially_copyable_v<TableView>);
static_assert(std::is_standard_layout_v<RowView>);
static_assert(std::is_trivially_copyable_v<RowView>);
static_assert(offsetof(TableView, structSize) == 0);
static_assert(offsetof(TableView, flags) == 4);
static_assert(offsetof(TableView, tableId) == 8);
static_assert(offsetof(TableView, bank) == 12);
static_assert(offsetof(TableView, revision) == 16);
static_assert(offsetof(TableView, rows) == 24);
static_assert(offsetof(TableView, rowCount) == 32);
static_assert(offsetof(TableView, rowSize) == 36);
static_assert(offsetof(TableView, reserved) == 40);
static_assert(TableViewRequiredSize == 40);
static_assert(sizeof(TableView) == 48);
static_assert(offsetof(RowView, structSize) == 0);
static_assert(offsetof(RowView, flags) == 4);
static_assert(offsetof(RowView, tableId) == 8);
static_assert(offsetof(RowView, bank) == 12);
static_assert(offsetof(RowView, revision) == 16);
static_assert(offsetof(RowView, row) == 24);
static_assert(offsetof(RowView, rowIndex) == 32);
static_assert(offsetof(RowView, rowSize) == 36);
static_assert(offsetof(RowView, reserved) == 40);
static_assert(RowViewRequiredSize == 40);
static_assert(sizeof(RowView) == 48);

}

struct DataTableServiceV1 {
	uint32_t                    serviceSize;
	uint32_t                    serviceVersion;
	DataTables::GetTableFn      getTable;
	DataTables::GetRowFn        getRow;
	DataTables::FindRowByIdFn   findRowById;
	DataTables::FindRowByCodeFn findRowByCode;
};

inline constexpr uint32_t DataTableServiceV1Version      = 1;
inline constexpr uint32_t DataTableServiceV1Size         = static_cast<uint32_t>(sizeof(DataTableServiceV1));
inline constexpr uint32_t DataTableServiceV1RequiredSize = static_cast<uint32_t>(offsetof(DataTableServiceV1, findRowByCode) + sizeof(DataTables::FindRowByCodeFn));

inline auto HasDataTableServiceV1Field(const DataTableServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == DataTableServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<DataTableServiceV1>);
static_assert(std::is_trivially_copyable_v<DataTableServiceV1>);
static_assert(offsetof(DataTableServiceV1, serviceSize) == 0);
static_assert(offsetof(DataTableServiceV1, serviceVersion) == 4);
static_assert(offsetof(DataTableServiceV1, getTable) == 8);
static_assert(offsetof(DataTableServiceV1, getRow) == 16);
static_assert(offsetof(DataTableServiceV1, findRowById) == 24);
static_assert(offsetof(DataTableServiceV1, findRowByCode) == 32);
static_assert(DataTableServiceV1RequiredSize == 40);
static_assert(sizeof(DataTableServiceV1) == 40);

}
