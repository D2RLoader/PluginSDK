#include <D2RLPlugin/api.h>
#include <cstdio>

static constexpr D2RL::PluginInfo ItemInteractionsPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "item-interactions-sample",
	.name        = "Item Interactions Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Logs semantic item activation events.",
	.flags       = D2RL::PluginFlags::Client,
};

static auto __cdecl OnItemInteraction(const D2RL::PluginContext* context, const D2RL::ItemInteractions::ItemInteractionEvent* event, void*) noexcept -> D2RL::ItemInteractions::Decision {
	if (context == nullptr || event == nullptr || event->structSize < D2RL::ItemInteractions::ItemInteractionEventRequiredSize) {
		return D2RL::ItemInteractions::Decision::Continue;
	}
	char message[192] {};
	std::snprintf(message, sizeof(message), "Activated item=%llu player=%llu container=%u cell=(%d,%d) input=%u modifiers=0x%X.", static_cast<unsigned long long>(event->item), static_cast<unsigned long long>(event->player), static_cast<uint32_t>(event->container), event->cellX, event->cellY, static_cast<uint32_t>(event->inputSource), event->modifiers);
	context->LogInfo(message);
	return D2RL::ItemInteractions::Decision::Continue;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &ItemInteractionsPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}
	const D2RL::ItemInteractionServiceV1* interactions = nullptr;
	if (context->QueryService(D2RL::ServiceId::ItemInteraction, D2RL::ItemInteractionServiceV1Version, &interactions) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasItemInteractionServiceV1Field(interactions, D2RL::ItemInteractionServiceV1RequiredSize)) {
		return false;
	}
	const D2RL::ItemInteractions::ItemInteractionListener listener {
		.structSize = D2RL::ItemInteractions::ItemInteractionListenerSize,
		.callback   = OnItemInteraction,
	};
	D2RL::ItemInteractions::ListenerHandle handle = D2RL::ItemInteractions::InvalidHandle;
	return interactions->registerListener(context, &listener, &handle) == D2RL::ItemInteractions::Result::Success;
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
