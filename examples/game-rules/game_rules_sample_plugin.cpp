#include <D2RLPlugin/api.h>
#include <cstdio>

static constexpr D2RL::PluginInfo GameRulesPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "game-rules-sample",
	.name        = "Game Rules Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Reports final item and skill rules.",
	.flags       = D2RL::PluginFlags::Shared,
};

static const D2RL::GameRuleServiceV1*  gameRules;
static const D2RL::InventoryServiceV1* inventory;
static const D2RL::ThreadServiceV1*    threads;

struct FirstItem {
	D2RL::ItemHandle handle = D2RL::InvalidItemHandle;
};

static auto __cdecl CaptureFirstItem(const D2RL::PluginContext*, const D2RL::Items::ItemInfo* item, void* userData) noexcept -> D2RL::Inventory::IterationAction {
	auto* first = static_cast<FirstItem*>(userData);
	if (first != nullptr && item != nullptr && item->structSize >= D2RL::Items::ItemInfoRequiredSize) {
		first->handle = item->handle;
	}
	return D2RL::Inventory::IterationAction::Stop;
}

static void __cdecl ReportRules(const D2RL::PluginContext* context, void*) noexcept {
	D2RL::PlayerHandle player = D2RL::InvalidPlayerHandle;
	if (inventory->getLocalPlayer(context, &player) != D2RL::Inventory::Result::Success) {
		context->LogWarn("The game-rules sample needs an authoritative local player.");
		return;
	}

	constexpr uint32_t SkillId          = 0;
	uint32_t           maxSkillLevel    = 0;
	bool               canAllocate      = false;
	const auto         skillResult      = gameRules->getRuntimeMaxSkillLevel(context, SkillId, &maxSkillLevel);
	const auto         allocationResult = gameRules->canAllocateSkill(context, player, SkillId, &canAllocate);

	FirstItem                         first;
	const D2RL::Inventory::ItemFilter filter {
		.structSize    = D2RL::Inventory::ItemFilterSize,
		.containerMask = D2RL::Items::ContainerBit(D2RL::Items::ItemContainer::Inventory) | D2RL::Items::ContainerBit(D2RL::Items::ItemContainer::Cube)
		               | D2RL::Items::ContainerBit(D2RL::Items::ItemContainer::PersonalStash),
	};
	(void)inventory->forEachInventoryItem(context, player, &filter, CaptureFirstItem, &first);

	uint32_t   maxSockets   = 0;
	uint32_t   maxStack     = 0;
	const auto socketResult = first.handle != D2RL::InvalidItemHandle ? gameRules->getMaxSockets(context, first.handle, &maxSockets) : D2RL::GameRules::Result::NotFound;
	const auto stackResult  = first.handle != D2RL::InvalidItemHandle ? gameRules->getTotalMaxStack(context, first.handle, &maxStack) : D2RL::GameRules::Result::NotFound;

	char message[224] {};
	std::snprintf(message,
		sizeof(message),
		"Rules: skill %u max=%u allocate=%s (results %u/%u), first item sockets=%u stack=%u (results %u/%u).",
		SkillId,
		maxSkillLevel,
		canAllocate ? "yes" : "no",
		static_cast<unsigned>(skillResult),
		static_cast<unsigned>(allocationResult),
		maxSockets,
		maxStack,
		static_cast<unsigned>(socketResult),
		static_cast<unsigned>(stackResult));
	context->LogInfo(message);
}

static auto GameRulesCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	return threads->runOnGameThread(command->plugin, ReportRules, nullptr) == D2RL::Threads::Result::Success ? D2RL::ConsoleCommandResult::Handled : D2RL::ConsoleCommandResult::Failed;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &GameRulesPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::GameRule, D2RL::GameRuleServiceV1Version, &gameRules) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasGameRuleServiceV1Field(gameRules, D2RL::GameRuleServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Inventory, D2RL::InventoryServiceV1Version, &inventory) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasInventoryServiceV1Field(inventory, D2RL::InventoryServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Thread, D2RL::ThreadServiceV1Version, &threads) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasThreadServiceV1Field(threads, D2RL::ThreadServiceV1RequiredSize)) {
		return false;
	}
	return context->RegisterConsoleCommand("game-rules-sample", GameRulesCommand, "Report final item and skill rules.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
