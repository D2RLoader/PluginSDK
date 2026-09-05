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

inline constexpr uint32_t HookQuerySize          = static_cast<uint32_t>(sizeof(HookQuery));
inline constexpr uint32_t HookQueryRequiredSize  = HookQuerySize;
inline constexpr uint32_t HookStatusSize         = static_cast<uint32_t>(sizeof(HookStatus));
inline constexpr uint32_t HookStatusRequiredSize = HookStatusSize;

using QueryHookStatusFn = Result(__cdecl*)(const PluginContext* context, const HookQuery* query, HookStatus* status) noexcept;

static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(ModificationState) == sizeof(uint32_t));
static_assert(sizeof(ModificationKind) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<HookQuery> && std::is_trivially_copyable_v<HookQuery>);
static_assert(std::is_standard_layout_v<HookStatus> && std::is_trivially_copyable_v<HookStatus>);
static_assert(sizeof(HookQuery) == 32);
static_assert(sizeof(HookStatus) == 96);

}

struct DiagnosticsService {
	static constexpr ServiceId Id         = ServiceId::Diagnostics;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                       serviceSize;
	uint32_t                       serviceVersion;
	Diagnostics::QueryHookStatusFn queryHookStatus;
};

inline constexpr uint32_t DiagnosticsServiceSize         = static_cast<uint32_t>(sizeof(DiagnosticsService));
inline constexpr uint32_t DiagnosticsServiceRequiredSize = DiagnosticsServiceSize;

inline auto HasDiagnosticsServiceField(const DiagnosticsService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == DiagnosticsService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<DiagnosticsService> && std::is_trivially_copyable_v<DiagnosticsService>);
static_assert(sizeof(DiagnosticsService) == 16);

}
