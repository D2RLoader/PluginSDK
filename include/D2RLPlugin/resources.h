#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Resources {

using RegistrationHandle = uint64_t;

inline constexpr RegistrationHandle InvalidHandle = 0;

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
};

enum class RegistrationState : uint32_t {
	Unknown = 0,
	Staged  = 1,
	Active  = 2,
	Closing = 3,
};

// Set structSize to ResourceRegistrationSize. path is a full normalized virtual
// path beginning with "data/". Keep it in either the normal
// ".../d2rloader/<plugin-id>/..." namespace or the panel-layout namespace
// "data/global/ui/layouts/<plugin-id>/...". Forward slashes are separators.
// D2RLoader copies path and bytes before registerResource returns, so the plugin
// may release both input buffers after the call. In v1, paths use
// printable ASCII and are limited to 512 characters. One resource is limited
// to 64 MiB, and one plugin load may register at most 256 MiB. flags must be
// zero in v1.
struct ResourceRegistration {
	uint32_t    structSize;
	uint32_t    flags;
	const char* path;
	const void* bytes;
	uint64_t    byteCount;
};

// Set structSize to RegistrationInfoSize. This reports read-only metadata for a
// registration owned by the calling plugin. D2RLoader owns the registration.
struct RegistrationInfo {
	uint32_t           structSize;
	uint32_t           flags;
	RegistrationHandle handle;
	uint64_t           ownerGeneration;
	uint64_t           byteCount;
	RegistrationState  state;
	uint32_t           reserved;
};

inline constexpr uint32_t ResourceRegistrationSize         = static_cast<uint32_t>(sizeof(ResourceRegistration));
inline constexpr uint32_t ResourceRegistrationRequiredSize = static_cast<uint32_t>(offsetof(ResourceRegistration, byteCount) + sizeof(uint64_t));
inline constexpr uint32_t RegistrationInfoSize             = static_cast<uint32_t>(sizeof(RegistrationInfo));
inline constexpr uint32_t RegistrationInfoRequiredSize     = static_cast<uint32_t>(offsetof(RegistrationInfo, reserved));

inline auto HasResourceRegistrationField(const ResourceRegistration* registration, uint32_t fieldEndOffset) noexcept -> bool {
	return registration != nullptr && registration->structSize >= fieldEndOffset;
}

inline auto HasRegistrationInfoField(const RegistrationInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->structSize >= fieldEndOffset;
}

using RegisterResourceFn    = Result(__cdecl*)(const PluginContext* context, const ResourceRegistration* registration, RegistrationHandle* handle) noexcept;
// Handles belong to one plugin load. Only that owner may inspect or unregister
// them. D2RCore also removes them automatically when the owner unloads.
using UnregisterResourceFn  = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle) noexcept;
using GetRegistrationInfoFn = Result(__cdecl*)(const PluginContext* context, RegistrationHandle handle, RegistrationInfo* info) noexcept;

static_assert(sizeof(RegistrationHandle) == sizeof(uint64_t));
static_assert(sizeof(Result) == sizeof(uint32_t));
static_assert(sizeof(RegistrationState) == sizeof(uint32_t));
static_assert(std::is_standard_layout_v<ResourceRegistration>);
static_assert(std::is_trivially_copyable_v<ResourceRegistration>);
static_assert(std::is_standard_layout_v<RegistrationInfo>);
static_assert(std::is_trivially_copyable_v<RegistrationInfo>);
static_assert(offsetof(ResourceRegistration, structSize) == 0);
static_assert(offsetof(ResourceRegistration, flags) == 4);
static_assert(offsetof(ResourceRegistration, path) == 8);
static_assert(offsetof(ResourceRegistration, bytes) == 16);
static_assert(offsetof(ResourceRegistration, byteCount) == 24);
static_assert(ResourceRegistrationRequiredSize == 32);
static_assert(sizeof(ResourceRegistration) == 32);
static_assert(offsetof(RegistrationInfo, structSize) == 0);
static_assert(offsetof(RegistrationInfo, flags) == 4);
static_assert(offsetof(RegistrationInfo, handle) == 8);
static_assert(offsetof(RegistrationInfo, ownerGeneration) == 16);
static_assert(offsetof(RegistrationInfo, byteCount) == 24);
static_assert(offsetof(RegistrationInfo, state) == 32);
static_assert(offsetof(RegistrationInfo, reserved) == 36);
static_assert(RegistrationInfoRequiredSize == 36);
static_assert(sizeof(RegistrationInfo) == 40);

}

struct ResourceServiceV1 {
	uint32_t                         serviceSize;
	uint32_t                         serviceVersion;
	Resources::RegisterResourceFn    registerResource;
	Resources::UnregisterResourceFn  unregisterResource;
	Resources::GetRegistrationInfoFn getRegistrationInfo;
};

inline constexpr uint32_t ResourceServiceV1Version      = 1;
inline constexpr uint32_t ResourceServiceV1Size         = static_cast<uint32_t>(sizeof(ResourceServiceV1));
inline constexpr uint32_t ResourceServiceV1RequiredSize = static_cast<uint32_t>(offsetof(ResourceServiceV1, getRegistrationInfo) + sizeof(Resources::GetRegistrationInfoFn));

inline auto HasResourceServiceV1Field(const ResourceServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == ResourceServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<ResourceServiceV1>);
static_assert(std::is_trivially_copyable_v<ResourceServiceV1>);
static_assert(offsetof(ResourceServiceV1, serviceSize) == 0);
static_assert(offsetof(ResourceServiceV1, serviceVersion) == 4);
static_assert(offsetof(ResourceServiceV1, registerResource) == 8);
static_assert(offsetof(ResourceServiceV1, unregisterResource) == 16);
static_assert(offsetof(ResourceServiceV1, getRegistrationInfo) == 24);
static_assert(ResourceServiceV1RequiredSize == 32);
static_assert(sizeof(ResourceServiceV1) == 32);

}
