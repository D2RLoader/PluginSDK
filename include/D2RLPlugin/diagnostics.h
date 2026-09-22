#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Diagnostics {

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unavailable     = 2,
	OwnerInactive   = 3,
	CallbackFault   = 4,
	BufferTooSmall  = 5,
};

enum class ModificationState : uint32_t {
	Unchanged = 0,
	Tracked   = 1,
	Untracked = 2,
};

enum class ModificationKind : uint32_t {
	Unknown    = 0,
	BytePatch  = 1,
	InlineHook = 2,
	Multiple   = 3,
};

enum class CallThroughState : uint32_t {
	Unknown = 0,
	No      = 1,
	Yes     = 2,
};

struct HookQuery {
	uint32_t    structSize;
	uint32_t    flags;
	uint64_t    rva;
	const void* expected;
	uint32_t    expectedSize;
	uint32_t    reserved;
};

// QueryHookStatus compares expectedSize live bytes at rva with expected. A
// changed range is Tracked when it overlaps a D2RLoader-managed plugin patch or
// inline hook. Untracked means the bytes changed without a known owner.
struct HookStatus {
	uint32_t          structSize;
	uint32_t          flags;
	ModificationState state;
	ModificationKind  kind;
	uint64_t          rva;
	uint32_t          size;
	uint32_t          ownerCount;
	char              ownerPluginId[64];
};

// ownerPluginId is filled only when one plugin owns the tracked range.
// ownerCount still reports how many distinct plugin owners overlap it.

// Each tracked entry describes one known patch or hook inside a changed range.
// Untracked entries cover changed bytes with no known owner. callThrough is Yes
// only when D2RLoader can prove that an inline hook has an executable pointer
// for calling the original code.
struct ModificationRange {
	uint32_t          structSize;
	uint32_t          flags;
	uint64_t          rva;
	uint32_t          size;
	ModificationState state;
	ModificationKind  kind;
	CallThroughState  callThrough;
	char              ownerPluginId[64];
};

inline constexpr uint32_t HookQuerySize                 = static_cast<uint32_t>(sizeof(HookQuery));
inline constexpr uint32_t HookQueryRequiredSize         = HookQuerySize;
inline constexpr uint32_t HookStatusSize                = static_cast<uint32_t>(sizeof(HookStatus));
inline constexpr uint32_t HookStatusRequiredSize        = HookStatusSize;
inline constexpr uint32_t ModificationRangeSize         = static_cast<uint32_t>(sizeof(ModificationRange));
inline constexpr uint32_t ModificationRangeRequiredSize = ModificationRangeSize;

using QueryHookStatusFn             = Result(__cdecl*)(const PluginContext* context, const HookQuery* query, HookStatus* status) noexcept;
// Call once with ranges set to null and capacity set to zero. count receives the
// required number of entries. BufferTooSmall means the query found entries.
using EnumerateModificationRangesFn = Result(__cdecl*)(const PluginContext* context, const HookQuery* query, ModificationRange* ranges, uint32_t capacity, uint32_t* count) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(ModificationState) == sizeof(uint32_t));
static_assert(sizeof(ModificationKind) == sizeof(uint32_t));
static_assert(sizeof(CallThroughState) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<HookQuery> && std::is_trivially_copyable_v<HookQuery>);
static_assert(std::is_standard_layout_v<HookStatus> && std::is_trivially_copyable_v<HookStatus>);
static_assert(std::is_standard_layout_v<ModificationRange> && std::is_trivially_copyable_v<ModificationRange>);
static_assert(sizeof(HookQuery) == 32);
static_assert(sizeof(HookStatus) == 96);
static_assert(sizeof(ModificationRange) == 96);

}

struct DiagnosticsService {
	static constexpr ServiceId Id         = ServiceId::Diagnostics;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                                   serviceSize;
	uint32_t                                   serviceVersion;
	Diagnostics::QueryHookStatusFn             queryHookStatus;
	Diagnostics::EnumerateModificationRangesFn enumerateModificationRanges;
};

inline constexpr uint32_t DiagnosticsServiceSize         = static_cast<uint32_t>(sizeof(DiagnosticsService));
inline constexpr uint32_t DiagnosticsServiceRequiredSize = 16;
inline constexpr uint32_t DiagnosticsServiceEnumerateModificationRangesFieldEnd
    = static_cast<uint32_t>(offsetof(DiagnosticsService, enumerateModificationRanges) + sizeof(Diagnostics::EnumerateModificationRangesFn));

inline auto HasDiagnosticsServiceField(const DiagnosticsService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == DiagnosticsService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<DiagnosticsService> && std::is_trivially_copyable_v<DiagnosticsService>);
static_assert(offsetof(DiagnosticsService, enumerateModificationRanges) == 16);
static_assert(DiagnosticsServiceEnumerateModificationRangesFieldEnd == 24);
static_assert(sizeof(DiagnosticsService) == 24);

}
