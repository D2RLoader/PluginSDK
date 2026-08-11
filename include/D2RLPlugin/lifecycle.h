#pragma once

#include <D2RLPlugin/version.h>
#include <cstddef>
#include <cstdint>

#define D2RL_PLUGIN_GET_INFO_EXPORT "D2RLoaderGetPluginInfo"
#define D2RL_PLUGIN_LOAD_EXPORT     "D2RLoaderLoadPlugin"
#define D2RL_PLUGIN_UNLOAD_EXPORT   "D2RLoaderUnloadPlugin"

namespace D2RL {

enum class PluginFlags : uint32_t {
	None          = 0,
	ModScopedOnly = 0x00000001U,
	NativeHooks   = 0x00000002U,
	Client        = 0x00000004U,
	Server        = 0x00000008U,
	Shared        = 0x0000000CU,
};

inline constexpr uint32_t PluginRoleMask  = 0x0000000CU;
inline constexpr uint32_t PluginKnownMask = 0x0000000FU;

enum class LoadScope : uint32_t {
	Mod    = 1,
	Global = 2,
};

constexpr auto FlagsValue(PluginFlags flags) noexcept -> uint32_t {
	return static_cast<uint32_t>(flags);
}

constexpr auto operator |(PluginFlags lhs, PluginFlags rhs) noexcept -> PluginFlags {
	return static_cast<PluginFlags>(FlagsValue(lhs) | FlagsValue(rhs));
}

constexpr auto operator &(PluginFlags lhs, PluginFlags rhs) noexcept -> PluginFlags {
	return static_cast<PluginFlags>(FlagsValue(lhs) & FlagsValue(rhs));
}

constexpr auto operator |=(PluginFlags& lhs, PluginFlags rhs) noexcept -> PluginFlags& {
	lhs = lhs | rhs;
	return lhs;
}

constexpr auto HasFlag(PluginFlags flags, PluginFlags flag) noexcept -> bool {
	const uint32_t value = FlagsValue(flag);
	return value != 0 && (FlagsValue(flags) & value) == value;
}

constexpr auto PluginRoleValue(PluginFlags flags) noexcept -> uint32_t {
	return FlagsValue(flags) & PluginRoleMask;
}

constexpr auto NormalizePluginFlagsForApiVersion(uint32_t apiVersion, PluginFlags flags) noexcept -> PluginFlags {
	if (apiVersion >= D2RL_PLUGIN_ROLES_API_VERSION) {
		return flags;
	}

	const uint32_t nonRoleFlags = FlagsValue(flags) & ~PluginRoleMask;
	return static_cast<PluginFlags>(nonRoleFlags | FlagsValue(PluginFlags::Shared));
}

constexpr auto HasValidPluginRole(PluginFlags flags) noexcept -> bool {
	const uint32_t role = PluginRoleValue(flags);
	return role == FlagsValue(PluginFlags::Client) || role == FlagsValue(PluginFlags::Server) || role == FlagsValue(PluginFlags::Shared);
}

constexpr auto HasOnlyKnownPluginFlags(PluginFlags flags) noexcept -> bool {
	return (FlagsValue(flags) & ~PluginKnownMask) == 0;
}

static_assert(PluginRoleValue(NormalizePluginFlagsForApiVersion(2, PluginFlags::None)) == FlagsValue(PluginFlags::Shared));
static_assert(PluginRoleValue(NormalizePluginFlagsForApiVersion(3, PluginFlags::None)) == FlagsValue(PluginFlags::None));
static_assert(HasFlag(NormalizePluginFlagsForApiVersion(2, PluginFlags::NativeHooks), PluginFlags::NativeHooks));
static_assert(HasFlag(PluginFlags::Shared, PluginFlags::Shared));
static_assert(!HasFlag(PluginFlags::Client, PluginFlags::Shared));

struct PluginInfo {
	uint32_t    infoSize;
	uint32_t    apiVersion;
	const char* id;
	const char* name;
	const char* version;
	const char* author;
	const char* description;
	PluginFlags flags;
	uint32_t    reserved[4];
};

inline constexpr uint32_t PluginInfoSize            = static_cast<uint32_t>(sizeof(PluginInfo));
inline constexpr uint32_t PluginInfoRequiredSize    = static_cast<uint32_t>(offsetof(PluginInfo, reserved));
inline constexpr uint32_t PluginInfoApiVersionSize  = static_cast<uint32_t>(offsetof(PluginInfo, apiVersion) + sizeof(uint32_t));
inline constexpr uint32_t PluginInfoDescriptionSize = static_cast<uint32_t>(offsetof(PluginInfo, description) + sizeof(const char*));
inline constexpr uint32_t PluginInfoFlagsSize       = static_cast<uint32_t>(offsetof(PluginInfo, flags) + sizeof(PluginFlags));

inline auto HasPluginInfoField(const PluginInfo* info, uint32_t fieldEndOffset) noexcept -> bool {
	return info != nullptr && info->infoSize >= fieldEndOffset;
}

}
