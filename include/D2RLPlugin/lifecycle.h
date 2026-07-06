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
};

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
	return FlagsValue(flags & flag) != 0;
}

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
