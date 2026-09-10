#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace Localization {

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unavailable     = 2,
	NotFound        = 3,
	BufferTooSmall  = 4,
	OwnerInactive   = 5,
	CallbackFault   = 6,
};

using GetStringByIdFn  = Result(__cdecl*)(const PluginContext* context, uint32_t stringId, char* output, uint32_t outputSize, uint32_t* requiredSize) noexcept;
using GetStringByKeyFn = Result(__cdecl*)(const PluginContext* context, const char* key, char* output, uint32_t outputSize, uint32_t* requiredSize) noexcept;

// Text is copied as UTF-8, including the trailing null byte. Pass a null or small
// buffer first to receive BufferTooSmall and the required byte count.
// Local keys search the namespace named by PluginInfo.id, without an added prefix.
// Use d2r:Key or d2rloader:Key for game or loader text.
// Numeric lookup accepts original game IDs only.
// Package JSON defines strings by Key; custom id fields are ignored.

static_assert(sizeof(Result) == sizeof(uint32_t));

}

struct LocalizationService {
	static constexpr ServiceId Id         = ServiceId::Localization;
	static constexpr uint32_t  AbiVersion = 2;

	uint32_t                       serviceSize;
	uint32_t                       serviceVersion;
	Localization::GetStringByIdFn  getStringById;
	Localization::GetStringByKeyFn getStringByKey;
};

inline constexpr uint32_t LocalizationServiceSize         = static_cast<uint32_t>(sizeof(LocalizationService));
inline constexpr uint32_t LocalizationServiceRequiredSize = LocalizationServiceSize;

inline auto HasLocalizationServiceField(const LocalizationService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == LocalizationService::AbiVersion && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<LocalizationService> && std::is_trivially_copyable_v<LocalizationService>);
static_assert(sizeof(LocalizationService) == 24);

}
