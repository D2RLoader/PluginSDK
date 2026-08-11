#pragma once

#include <D2RLPlugin/inventory.h>
#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

namespace GameRules {

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	Unavailable     = 3,
	NotFound        = 4,
	Busy            = 5,
	OwnerInactive   = 6,
	StaleHandle     = 7,
	CallbackFault   = 8,
};

using GetMaxSocketsFn           = Result(__cdecl*)(const PluginContext* context, ItemHandle item, uint32_t* maximum) noexcept;
using GetTotalMaxStackFn        = Result(__cdecl*)(const PluginContext* context, ItemHandle item, uint32_t* maximum) noexcept;
using GetRuntimeMaxSkillLevelFn = Result(__cdecl*)(const PluginContext* context, uint32_t skillId, uint32_t* maximum) noexcept;
using CanAllocateSkillFn        = Result(__cdecl*)(const PluginContext* context, PlayerHandle player, uint32_t skillId, bool* allowed) noexcept;

// These return the final rules after table data and D2RLoader changes. Ordinary
// reads run on the UI thread or in a scheduled game-thread callback.
// canAllocateSkill needs the game-thread callback and only checks the result; it
// never spends a point.

static_assert(sizeof(Result) == sizeof(uint32_t));

}

struct GameRuleServiceV1 {
	uint32_t                             serviceSize;
	uint32_t                             serviceVersion;
	GameRules::GetMaxSocketsFn           getMaxSockets;
	GameRules::GetTotalMaxStackFn        getTotalMaxStack;
	GameRules::GetRuntimeMaxSkillLevelFn getRuntimeMaxSkillLevel;
	GameRules::CanAllocateSkillFn        canAllocateSkill;
};

inline constexpr uint32_t GameRuleServiceV1Version      = 1;
inline constexpr uint32_t GameRuleServiceV1Size         = static_cast<uint32_t>(sizeof(GameRuleServiceV1));
inline constexpr uint32_t GameRuleServiceV1RequiredSize = GameRuleServiceV1Size;

inline auto HasGameRuleServiceV1Field(const GameRuleServiceV1* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceVersion == GameRuleServiceV1Version && service->serviceSize >= fieldEndOffset;
}

static_assert(std::is_standard_layout_v<GameRuleServiceV1> && std::is_trivially_copyable_v<GameRuleServiceV1>);
static_assert(sizeof(GameRuleServiceV1) == 40);

}
