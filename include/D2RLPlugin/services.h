#pragma once

#include <cstdint>

namespace D2RL {

struct PluginContext;

// These numeric ids are permanent parts of the ABI. A named service may still
// be unavailable when the running D2RLoader does not implement that version.
enum class ServiceId : uint32_t {
	Lifecycle    = 1,
	Resource     = 2,
	CustomTable  = 3,
	Panel        = 4,
	Inventory    = 5,
	Network      = 6,
	Input        = 7,
	DataTable    = 8,
	SharedEvent  = 9,
	Diagnostics  = 10,
	GameRule     = 11,
	Widget       = 12,
	Thread       = 13,
	Localization = 14,
	Item         = 15,
};

enum class ServiceQueryResult : uint32_t {
	Success            = 0,
	InvalidArgument    = 1,
	UnknownService     = 2,
	UnsupportedVersion = 3,
	Unavailable        = 4,
	OwnerInactive      = 5,
};

using QueryServiceFn = ServiceQueryResult(__cdecl*)(const PluginContext* context, ServiceId serviceId, uint32_t serviceVersion, const void** service) noexcept;

static_assert(sizeof(ServiceId) == sizeof(uint32_t));
static_assert(sizeof(ServiceQueryResult) == sizeof(uint32_t));

}
