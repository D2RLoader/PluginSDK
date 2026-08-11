#include <D2RLPlugin/api.h>
#include <array>
#include <cstdio>

static constexpr D2RL::PluginInfo ItemTransactionPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "item-transaction-sample",
	.name        = "Item Transaction Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Trades three El runes for one configured item.",
	.flags       = D2RL::PluginFlags::Shared,
};

static const D2RL::InventoryServiceV1* inventory;
static const D2RL::ItemServiceV1*      items;
static const D2RL::ThreadServiceV1*    threads;

struct RuneSearch {
	std::array<D2RL::ItemHandle, 3> handles {};
	uint32_t                        count = 0;
};

static auto __cdecl FindElRunes(const D2RL::PluginContext*, const D2RL::Items::ItemInfo* item, void* userData) noexcept -> D2RL::Inventory::IterationAction {
	auto* search = static_cast<RuneSearch*>(userData);
	if (search == nullptr || item == nullptr || item->structSize < D2RL::Items::ItemInfoRequiredSize) {
		return D2RL::Inventory::IterationAction::Stop;
	}

	if (item->code == D2RL::Items::MakeItemCode("r01") && search->count < search->handles.size()) {
		search->handles[search->count++] = item->handle;
	}
	return search->count == search->handles.size() ? D2RL::Inventory::IterationAction::Stop : D2RL::Inventory::IterationAction::Continue;
}

static auto MakeReward(const D2RL::Items::PropertySpec* properties) noexcept -> D2RL::Items::ItemCreateSpec {
	return {
		.structSize      = D2RL::Items::ItemCreateSpecSize,
		.code            = D2RL::Items::MakeItemCode("cap"),
		.quality         = D2RL::Items::Quality::Normal,
		.qualityRecordId = D2RL::Items::RandomQualityRecord,
		.itemLevel       = 12,
		.seedMode        = D2RL::Items::SeedMode::Random,
		.quantity        = D2RL::Items::DefaultValue,
		.durability      = D2RL::Items::DefaultValue,
		.socketCount     = 2,
		.stateFlags      = D2RL::Items::ItemStateIdentified | D2RL::Items::ItemStateEthereal,
		.propertyCount   = 1,
		.properties      = properties,
		.destination     = {
							.structSize = D2RL::Items::ItemDestinationSize,
							.container  = D2RL::Items::ItemContainer::Inventory,
							.placement  = D2RL::Items::Placement::Automatic,
							},
	};
}

static void __cdecl TradeRunes(const D2RL::PluginContext* context, void*) noexcept {
	D2RL::PlayerHandle player = D2RL::InvalidPlayerHandle;
	if (inventory->getLocalPlayer(context, &player) != D2RL::Inventory::Result::Success) {
		context->LogWarn("The item sample could not resolve the authoritative player.");
		return;
	}

	RuneSearch                        search;
	const D2RL::Inventory::ItemFilter filter {
		.structSize    = D2RL::Inventory::ItemFilterSize,
		.containerMask = D2RL::Items::ContainerBit(D2RL::Items::ItemContainer::Inventory),
	};
	const auto searchResult = inventory->forEachInventoryItem(context, player, &filter, FindElRunes, &search);
	if (searchResult != D2RL::Inventory::Result::Success || search.count != search.handles.size()) {
		context->LogWarn("Put three El runes in the normal inventory, then run item-sample-trade again.");
		return;
	}

	std::array<D2RL::Items::TransactionInput, 3> inputs {};
	for (size_t index = 0; index < inputs.size(); ++index) {
		inputs[index] = {
			.item               = search.handles[index],
			.quantity           = 1,
			.socketedItemPolicy = D2RL::Items::SocketedItemPolicy::RejectIfNotEmpty,
		};
	}

	// Properties.txt *Id 0 is the normal armor-class property. Matching minimum
	// and maximum values ask D2R for exactly +25 Defense.
	const D2RL::Items::PropertySpec   defense    = D2RL::Items::MakeExactProperty(0, 25);
	const D2RL::Items::ItemCreateSpec reward     = MakeReward(&defense);
	D2RL::ItemHandle                  outputItem = D2RL::InvalidItemHandle;
	const D2RL::Items::Transaction    transaction {
		.structSize     = D2RL::Items::TransactionSize,
		.player         = player,
		.inputCount     = static_cast<uint32_t>(inputs.size()),
		.outputCount    = 1,
		.inputs         = inputs.data(),
		.outputs        = &reward,
		.outputItems    = &outputItem,
		.outputCapacity = 1,
	};
	D2RL::Items::TransactionResult result {
		.structSize = D2RL::Items::TransactionResultSize,
	};
	if (items->executeTransaction(context, &transaction, &result) != D2RL::Items::Result::Success) {
		context->LogWarn("The exchange failed. Its inputs and any staged output were rolled back.");
		return;
	}

	D2RL::Items::ItemInfo info {
		.structSize = D2RL::Items::ItemInfoSize,
	};
	if (items->getItemInfo(context, outputItem, &info) != D2RL::Items::Result::Success) {
		context->LogInfo("The atomic item exchange committed.");
		return;
	}
	char message[160] {};
	std::snprintf(message,
		sizeof(message),
		"Exchange committed: item=%08X sockets=%u ethereal=%s.",
		info.code,
		info.socketCount,
		(info.stateFlags & D2RL::Items::ItemStateEthereal) != 0 ? "yes" : "no");
	context->LogInfo(message);
}

static auto ItemTradeCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	const auto queued = threads->runOnGameThread(command->plugin, TradeRunes, nullptr);
	if (queued != D2RL::Threads::Result::Success) {
		command->plugin->WriteConsoleMessage("The exchange must run in a local game or on the TCP/IP host.");
		return D2RL::ConsoleCommandResult::Failed;
	}
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &ItemTransactionPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Inventory, D2RL::InventoryServiceV1Version, &inventory) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasInventoryServiceV1Field(inventory, D2RL::InventoryServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Item, D2RL::ItemServiceV1Version, &items) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasItemServiceV1Field(items, D2RL::ItemServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Thread, D2RL::ThreadServiceV1Version, &threads) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasThreadServiceV1Field(threads, D2RL::ThreadServiceV1RequiredSize)) {
		return false;
	}
	return context->RegisterConsoleCommand("item-sample-trade", ItemTradeCommand, "Trade three inventory El runes for a configured cap.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
