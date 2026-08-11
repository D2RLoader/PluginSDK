#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Threads {

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unavailable     = 2,
	Busy            = 3,
	OwnerInactive   = 4,
	CallbackFault   = 5,
};

using Callback = void(__cdecl*)(const PluginContext* context, void* userData) noexcept;
using RunFn    = Result(__cdecl*)(const PluginContext* context, Callback callback, void* userData) noexcept;

// Both calls queue work and return right away. UI callbacks run during a later
// root-widget update. Game callbacks run during a later authoritative game
// update; this is the only supported place for item mutation and native item
// access. A remote TCP/IP client has no local authoritative game thread, so
// runOnGameThread returns Unavailable there. Unload discards all queued work,
// and a session change also discards queued game work.

static_assert(sizeof(Result) == sizeof(uint32_t));

}

struct ThreadServiceV1 {
	uint32_t       serviceSize;
	uint32_t       serviceVersion;
	Threads::RunFn runOnUiThread;
	Threads::RunFn runOnGameThread;
};

inline constexpr uint32_t ThreadServiceV1Version      = 1;
inline constexpr uint32_t ThreadServiceV1Size         = static_cast<uint32_t>(sizeof(ThreadServiceV1));
inline constexpr uint32_t ThreadServiceV1RequiredSize = ThreadServiceV1Size;

inline auto HasThreadServiceV1Field(const ThreadServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ThreadServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ThreadServiceV1> && std::is_trivially_copyable_v<ThreadServiceV1>);
static_assert(sizeof(ThreadServiceV1) == 24);

}
