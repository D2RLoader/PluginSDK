#pragma once

#include <D2RLPlugin/services.h>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace D2R::Game {

struct Client;

}

namespace D2RL {

struct PluginContext;

namespace Encounters {

using ProviderHandle = uint64_t;

inline constexpr ProviderHandle InvalidProviderHandle               = 0;
inline constexpr uint32_t       RequestForced                       = 1U << 0U;
inline constexpr uint32_t       HeraldDecisionReplace               = 1U << 0U;
inline constexpr uint32_t       HeraldDecisionOverrideTreasureClass = 1U << 1U;
inline constexpr uint32_t       HeraldDecisionOverrideBossHealth    = 1U << 2U;
inline constexpr uint32_t       HeraldDecisionOverrideModifiers     = 1U << 3U;
inline constexpr uint32_t       BossDecisionApplyHeraldModifiers    = 1U << 0U;
inline constexpr uint8_t        TestSpawnHealthBoostCapped          = 1U << 0U;

enum class Result : uint32_t {
	Success         = 0,
	InvalidArgument = 1,
	Unsupported     = 2,
	NotFound        = 3,
	Busy            = 4,
	OwnerInactive   = 5,
	OwnerMismatch   = 6,
	CallbackFault   = 7,
	Unavailable     = 8,
	NotInGame       = 9,
	NotTerrorZone   = 10,
	NoHeraldTier    = 11,
	SpawnFailed     = 12,
};

struct HeraldRequest {
	uint32_t structSize;
	uint32_t flags;
	uint32_t act;
	int32_t  heraldTier;
	int32_t  levelId;
	int32_t  originalMonsterIndex;
	uint32_t reserved[2];
};

// Added modifiers only. Boss-native skills and existing modifiers are retained.
// Modifier bits are monumod IDs. Aura bits follow AuraSkillIds below.
inline constexpr int32_t  AuraSkillIds[] = { 98, 368, 108, 365, 123, 122, 369, 113, 115, 428 };
inline constexpr uint32_t SupportedModifierMask
    = (1U << 5) | (1U << 6) | (1U << 7) | (1U << 8) | (1U << 9) | (1U << 17) | (1U << 18) | (1U << 25) | (1U << 26) | (1U << 27) | (1U << 28) | (1U << 29) | (1U << 30);
inline constexpr uint32_t SupportedAuraMask = (1U << 10) - 1;

struct ModifierPolicy {
	uint32_t allowedModifierMask;
	uint32_t minModifiers;
	uint32_t maxModifiers;
	uint32_t allowedAuraMask;
	uint32_t minAuras;
	uint32_t maxAuras;
	uint32_t maxAuraLevel;       // 0 keeps the native level formula (maximum 99).
	int32_t  damageBoostPercent; // Added tier damage; 0 disables it.
};

struct HeraldDecision {
	uint32_t       structSize;
	uint32_t       flags;
	int32_t        replacementMonsterIndex;
	uint32_t       minionCount;
	int32_t        minionMonsterIndices[3];
	int32_t        treasureClassMonsterIndex;
	int32_t        bossHealthBoostPercent;
	uint32_t       reserved;
	ModifierPolicy modifiers;
};

struct BossRequest {
	uint32_t structSize;
	uint32_t flags;
	int32_t  monsterIndex;
	int32_t  heraldTier;
	int32_t  levelId;
	uint32_t difficulty;
	uint32_t reserved[2];
};

struct BossDecision {
	uint32_t       structSize;
	uint32_t       flags;
	uint32_t       cloneCount;
	uint32_t       reserved;
	uint64_t       reserved2[2];
	ModifierPolicy modifiers;
};

using DecideHeraldFn = Result(__cdecl*)(const PluginContext* context, const HeraldRequest* request, HeraldDecision* decision, void* userData) noexcept;
using DecideBossFn   = Result(__cdecl*)(const PluginContext* context, const BossRequest* request, BossDecision* decision, void* userData) noexcept;

struct ProviderRegistration {
	uint32_t       structSize;
	uint32_t       flags;
	DecideHeraldFn decideHerald;
	DecideBossFn   decideBoss;
	void*          userData;
	uint64_t       reserved[2];
};

enum class TestAction : uint32_t {
	HeraldReplacement = 1,
	BossClones        = 2,
	HeraldBossGroup   = 3,
};

struct TestSpawnRequest {
	uint32_t   structSize;
	TestAction action;
	int32_t    monsterIndex;
	uint32_t   count;
	uint32_t   act;
	uint32_t   reserved[3];
};

struct TestSpawnResult {
	uint32_t structSize;
	uint32_t requestedCount;
	uint32_t spawnedCount;
	int32_t  heraldTier;
	int32_t  levelId;
	int32_t  monsterIndex;
	uint16_t monsterDataFlags;
	uint8_t  uniqueModifierCount;
	uint8_t  flags;
	uint32_t uniqueModifierMask;
};

inline constexpr uint32_t HeraldRequestSize        = static_cast<uint32_t>(sizeof(HeraldRequest));
inline constexpr uint32_t HeraldDecisionSize       = static_cast<uint32_t>(sizeof(HeraldDecision));
inline constexpr uint32_t BossRequestSize          = static_cast<uint32_t>(sizeof(BossRequest));
inline constexpr uint32_t BossDecisionSize         = static_cast<uint32_t>(sizeof(BossDecision));
inline constexpr uint32_t ProviderRegistrationSize = static_cast<uint32_t>(sizeof(ProviderRegistration));
inline constexpr uint32_t TestSpawnRequestSize     = static_cast<uint32_t>(sizeof(TestSpawnRequest));
inline constexpr uint32_t TestSpawnResultSize      = static_cast<uint32_t>(sizeof(TestSpawnResult));

using RegisterProviderFn   = Result(__cdecl*)(const PluginContext* context, const ProviderRegistration* registration, ProviderHandle* handle) noexcept;
using UnregisterProviderFn = Result(__cdecl*)(const PluginContext* context, ProviderHandle handle) noexcept;
using RunTestSpawnFn       = Result(__cdecl*)(const PluginContext* context, D2R::Game::Client* client, const TestSpawnRequest* request, TestSpawnResult* result) noexcept;

}

struct EncounterService {
	static constexpr ServiceId Id         = ServiceId::Encounter;
	static constexpr uint32_t  AbiVersion = 1;

	uint32_t                         serviceSize;
	uint32_t                         serviceVersion;
	Encounters::RegisterProviderFn   registerProvider;
	Encounters::UnregisterProviderFn unregisterProvider;
	Encounters::RunTestSpawnFn       runTestSpawn;
};

inline constexpr uint32_t EncounterServiceSize         = static_cast<uint32_t>(sizeof(EncounterService));
inline constexpr uint32_t EncounterServiceRequiredSize = static_cast<uint32_t>(offsetof(EncounterService, runTestSpawn) + sizeof(Encounters::RunTestSpawnFn));

inline auto HasEncounterServiceField(const EncounterService* service, uint32_t fieldEndOffset) noexcept -> bool {
	return service != nullptr && service->serviceSize >= fieldEndOffset;
}

static_assert(sizeof(Encounters::Result) == sizeof(uint32_t));
static_assert(sizeof(Encounters::HeraldRequest) == 32);
static_assert(sizeof(Encounters::ModifierPolicy) == 32);
static_assert(sizeof(Encounters::HeraldDecision) == 72);
static_assert(sizeof(Encounters::BossRequest) == 32);
static_assert(sizeof(Encounters::BossDecision) == 64);
static_assert(sizeof(Encounters::ProviderRegistration) == 48);
static_assert(sizeof(Encounters::TestSpawnRequest) == 32);
static_assert(sizeof(Encounters::TestSpawnResult) == 32);
static_assert(offsetof(Encounters::HeraldDecision, treasureClassMonsterIndex) == 28);
static_assert(offsetof(Encounters::HeraldDecision, bossHealthBoostPercent) == 32);
static_assert(offsetof(Encounters::TestSpawnResult, monsterDataFlags) == 24);
static_assert(offsetof(Encounters::TestSpawnResult, uniqueModifierMask) == 28);
static_assert(sizeof(EncounterService) == 32);
static_assert(std::is_standard_layout_v<Encounters::ProviderRegistration>);
static_assert(std::is_trivially_copyable_v<Encounters::ProviderRegistration>);

}
