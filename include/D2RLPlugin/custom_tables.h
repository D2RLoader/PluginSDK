#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace CustomTables {

using TableHandle = uint64_t;

inline constexpr TableHandle InvalidHandle = 0;

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
	TooLarge        = 11,
	StaleRevision   = 12,
	BufferTooSmall  = 13,
};

// Use All only as a registration mask. getTableInfo and copyRows need one exact
// bank: Base or Rotw.
enum class TableBank : uint32_t {
	None = 0,
	Base = 1,
	Rotw = 2,
	All  = 3,
};

enum class ColumnType : uint32_t {
	Ascii = 1,
	Byte  = 2,
	Word  = 3,
	Dword = 4,
};

enum class TableState : uint32_t {
	Unknown = 0,
	Staged  = 1,
	Active  = 2,
	Loading = 3,
	Ready   = 4,
	Failed  = 5,
	Closing = 6,
};

// Set structSize to ColumnDefinitionSize. D2RLoader copies the column and its
// name before registerTable returns. For Ascii, length is the full destination
// capacity, including the trailing NUL. Leave it zero for Byte, Word, and
// Dword. flags and reserved are also zero in v1. Names use 1-64 printable ASCII
// characters and are matched without case; changing capitalization does not
// make a duplicate name unique.
struct ColumnDefinition {
	uint32_t    structSize;
	uint32_t    flags;
	const char* name;
	ColumnType  type;
	uint32_t    offset;
	uint32_t    length;
	uint32_t    reserved;
};

// Set structSize to TableRegistrationSize and columnStride to
// ColumnDefinitionSize for a v1 array. name is one file-name segment such as
// "charminv", not a path or extension. It uses 1-64 lowercase ASCII characters,
// starts with a letter or digit, and may also contain '.', '_', or '-'. D2RCore
// expands it to "d2rloader/<plugin-id>/<name>". Put Rotw resources below
// data/global/excel/ and Base resources below data/global/excel/base/.
//
// TXT input is strict tab-separated text. The printable-ASCII header must be
// unique and contain every registered column once. Every nonblank data row
// needs the same cell count, printable-ASCII values, and a final line feed.
// Numeric cells are empty (zero) or unsigned decimal values that fit the field.
// A header by itself is a valid empty table.
//
// D2RCore copies the name, columns, and every column name. columnStride lets a
// later SDK append fields without changing how a v1 array is read. flags is zero
// in v1. The v1 limits are 349 non-overlapping columns, a 1 MiB row, and a
// 4096-byte column stride.
struct TableRegistration {
	uint32_t                structSize;
	uint32_t                flags;
	const char*             name;
	TableBank               banks;
	uint32_t                rowSize;
	const ColumnDefinition* columns;
	uint32_t                columnCount;
	uint32_t                columnStride;
};

// Set structSize to TableInfoSize before calling getTableInfo. It reports one
// exact bank. revision changes when that bank publishes a new snapshot or a
// load fails. D2RCore owns the rows; copyRows is the only way to receive them.
struct TableInfo {
	uint32_t    structSize;
	uint32_t    flags;
	TableHandle handle;
	uint64_t    ownerGeneration;
	uint64_t    revision;
	TableBank   bank;
	TableState  state;
	uint32_t    rowSize;
	uint32_t    rowCount;
	uint64_t    byteCount;
	uint64_t    reserved;
};

inline constexpr uint32_t ColumnDefinitionSize          = static_cast<uint32_t>(sizeof(ColumnDefinition));
inline constexpr uint32_t ColumnDefinitionRequiredSize  = static_cast<uint32_t>(offsetof(ColumnDefinition, reserved) + sizeof(uint32_t));
inline constexpr uint32_t TableRegistrationSize         = static_cast<uint32_t>(sizeof(TableRegistration));
inline constexpr uint32_t TableRegistrationRequiredSize = static_cast<uint32_t>(offsetof(TableRegistration, columnStride) + sizeof(uint32_t));
inline constexpr uint32_t TableInfoSize                 = static_cast<uint32_t>(sizeof(TableInfo));
inline constexpr uint32_t TableInfoRequiredSize         = static_cast<uint32_t>(offsetof(TableInfo, reserved));

inline auto HasColumnDefinitionField(const ColumnDefinition* column, uint32_t fieldEndOffset) noexcept -> bool {
	return column != nullptr && column->structSize >= fieldEndOffset;
}

inline auto HasTableRegistrationField(const TableRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasTableInfoField(const TableInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

using RegisterTableFn   = Result(__cdecl*)(const PluginContext* context, const TableRegistration* registration, TableHandle* handle) noexcept;
using UnregisterTableFn = Result(__cdecl*)(const PluginContext* context, TableHandle handle) noexcept;
using GetTableInfoFn    = Result(__cdecl*)(const PluginContext* context, TableHandle handle, TableBank bank, TableInfo* info) noexcept;
// expectedRevision must match the Ready snapshot from getTableInfo, and
// outputByteCount must cover TableInfo::byteCount. If a reload lands between
// the calls, copyRows returns StaleRevision without writing a partial result. A
// Ready empty table accepts a null output with zero bytes.
using CopyRowsFn        = Result(__cdecl*)(const PluginContext* context, TableHandle handle, TableBank bank, uint64_t expectedRevision, void* output, uint64_t outputByteCount) noexcept;

static_assert(sizeof(TableHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(TableBank) == sizeof(uint32_t));
static_assert(sizeof(ColumnType) == sizeof(uint32_t));
static_assert(sizeof(TableState) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ColumnDefinition>);
static_assert(std::is_trivially_copyable_v<ColumnDefinition>);
static_assert(std::is_standard_layout_v<TableRegistration>);
static_assert(std::is_trivially_copyable_v<TableRegistration>);
static_assert(std::is_standard_layout_v<TableInfo>);
static_assert(std::is_trivially_copyable_v<TableInfo>);
static_assert(offsetof(ColumnDefinition, structSize) == 0);
static_assert(offsetof(ColumnDefinition, flags) == 4);
static_assert(offsetof(ColumnDefinition, name) == 8);
static_assert(offsetof(ColumnDefinition, type) == 16);
static_assert(offsetof(ColumnDefinition, offset) == 20);
static_assert(offsetof(ColumnDefinition, length) == 24);
static_assert(offsetof(ColumnDefinition, reserved) == 28);
static_assert(ColumnDefinitionRequiredSize == 32);
static_assert(sizeof(ColumnDefinition) == 32);
static_assert(offsetof(TableRegistration, structSize) == 0);
static_assert(offsetof(TableRegistration, flags) == 4);
static_assert(offsetof(TableRegistration, name) == 8);
static_assert(offsetof(TableRegistration, banks) == 16);
static_assert(offsetof(TableRegistration, rowSize) == 20);
static_assert(offsetof(TableRegistration, columns) == 24);
static_assert(offsetof(TableRegistration, columnCount) == 32);
static_assert(offsetof(TableRegistration, columnStride) == 36);
static_assert(TableRegistrationRequiredSize == 40);
static_assert(sizeof(TableRegistration) == 40);
static_assert(offsetof(TableInfo, structSize) == 0);
static_assert(offsetof(TableInfo, flags) == 4);
static_assert(offsetof(TableInfo, handle) == 8);
static_assert(offsetof(TableInfo, ownerGeneration) == 16);
static_assert(offsetof(TableInfo, revision) == 24);
static_assert(offsetof(TableInfo, bank) == 32);
static_assert(offsetof(TableInfo, state) == 36);
static_assert(offsetof(TableInfo, rowSize) == 40);
static_assert(offsetof(TableInfo, rowCount) == 44);
static_assert(offsetof(TableInfo, byteCount) == 48);
static_assert(offsetof(TableInfo, reserved) == 56);
static_assert(TableInfoRequiredSize == 56);
static_assert(sizeof(TableInfo) == 64);

}

struct CustomTableServiceV1 {
	uint32_t                        serviceSize;
	uint32_t                        serviceVersion;
	CustomTables::RegisterTableFn   registerTable;
	CustomTables::UnregisterTableFn unregisterTable;
	CustomTables::GetTableInfoFn    getTableInfo;
	CustomTables::CopyRowsFn        copyRows;
};

inline constexpr uint32_t CustomTableServiceV1Version      = 1;
inline constexpr uint32_t CustomTableServiceV1Size         = static_cast<uint32_t>(sizeof(CustomTableServiceV1));
inline constexpr uint32_t CustomTableServiceV1RequiredSize = static_cast<uint32_t>(offsetof(CustomTableServiceV1, copyRows) + sizeof(CustomTables::CopyRowsFn));

inline auto HasCustomTableServiceV1Field(const CustomTableServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == CustomTableServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<CustomTableServiceV1>);
static_assert(std::is_trivially_copyable_v<CustomTableServiceV1>);
static_assert(offsetof(CustomTableServiceV1, serviceSize) == 0);
static_assert(offsetof(CustomTableServiceV1, serviceVersion) == 4);
static_assert(offsetof(CustomTableServiceV1, registerTable) == 8);
static_assert(offsetof(CustomTableServiceV1, unregisterTable) == 16);
static_assert(offsetof(CustomTableServiceV1, getTableInfo) == 24);
static_assert(offsetof(CustomTableServiceV1, copyRows) == 32);
static_assert(CustomTableServiceV1RequiredSize == 40);
static_assert(sizeof(CustomTableServiceV1) == 40);

}
