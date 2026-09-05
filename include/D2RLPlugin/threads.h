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

struct ThreadService {
	static constexpr ServiceId Id         = ServiceId::Thread;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t       serviceSize;
	uint32_t       serviceVersion;
	Threads::RunFn runOnUiThread;
	Threads::RunFn runOnGameThread;
};

inline constexpr uint32_t ThreadServiceSize         = static_cast<uint32_t>(sizeof(ThreadService));
inline constexpr uint32_t ThreadServiceRequiredSize = ThreadServiceSize;

inline auto HasThreadServiceField(const ThreadService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ThreadService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ThreadService> && std::is_trivially_copyable_v<ThreadService>);
static_assert(sizeof(ThreadService) == 24);

}
