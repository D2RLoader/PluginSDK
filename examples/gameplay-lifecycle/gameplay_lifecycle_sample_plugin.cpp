#include <D2RLPlugin/api.h>
#include <array>
#include <cstdio>

static constexpr D2RL::PluginInfo GameplayLifecyclePluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "gameplay-lifecycle-sample",
	.name        = "Gameplay Lifecycle Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Reports game, player, location, level-up, quest, and resurrection events.",
	.flags       = D2RL::PluginFlags::Shared,
};

static auto EventName(D2RL::Lifecycle::GameplayEventKind kind) noexcept -> const char* {
	switch (kind) {
		case D2RL::Lifecycle::GameplayEventKind::GameJoined:         return "game joined";
		case D2RL::Lifecycle::GameplayEventKind::GameLeft:           return "game left";
		case D2RL::Lifecycle::GameplayEventKind::LocalPlayerReady:   return "local player ready";
		case D2RL::Lifecycle::GameplayEventKind::ActChanged:         return "act changed";
		case D2RL::Lifecycle::GameplayEventKind::LevelChanged:       return "level changed";
		case D2RL::Lifecycle::GameplayEventKind::PlayerResurrected:  return "player resurrected";
		case D2RL::Lifecycle::GameplayEventKind::PlayerLevelChanged: return "player level changed";
		case D2RL::Lifecycle::GameplayEventKind::QuestCompleted:     return "quest completed";
		default:                                                     return "unknown";
	}
}

static void __cdecl OnGameplayEvent(const D2RL::PluginContext* context, const D2RL::Lifecycle::GameplayEvent* event, void*) noexcept {
	if (context == nullptr || !D2RL::Lifecycle::HasGameplayEventField(event, D2RL::Lifecycle::GameplayEventRequiredSize)) {
		return;
	}
	char       message[256] {};
	const auto session = static_cast<unsigned long long>(event->sessionGeneration);
	std::snprintf(message, sizeof(message), "Lifecycle: %s, player=%u, session=%llu, previous=%d, current=%d, difficulty=%u, quest-row=%u.", EventName(event->kind), event->playerId, session, event->previousValue, event->currentValue, event->difficulty, event->questRecordId);
	context->LogInfo(message);
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &GameplayLifecyclePluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	const D2RL::LifecycleServiceV1* lifecycle = nullptr;
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Lifecycle, D2RL::LifecycleServiceV1Version, &lifecycle) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasLifecycleServiceV1Field(lifecycle, D2RL::LifecycleServiceV1RequiredSize)) {
		return false;
	}

	static constexpr std::array Kinds {
		D2RL::Lifecycle::GameplayEventKind::GameJoined,
		D2RL::Lifecycle::GameplayEventKind::GameLeft,
		D2RL::Lifecycle::GameplayEventKind::LocalPlayerReady,
		D2RL::Lifecycle::GameplayEventKind::ActChanged,
		D2RL::Lifecycle::GameplayEventKind::LevelChanged,
		D2RL::Lifecycle::GameplayEventKind::PlayerResurrected,
		D2RL::Lifecycle::GameplayEventKind::PlayerLevelChanged,
		D2RL::Lifecycle::GameplayEventKind::QuestCompleted,
	};
	for (const auto kind : Kinds) {
		const D2RL::Lifecycle::GameplayEventListener listener {
			.structSize = D2RL::Lifecycle::GameplayEventListenerSize,
			.kind       = kind,
			.callback   = OnGameplayEvent,
		};
		D2RL::Lifecycle::ListenerHandle handle = D2RL::Lifecycle::InvalidHandle;
		if (lifecycle->registerGameplayEventListener(context, &listener, &handle) != D2RL::Lifecycle::Result::Success) {
			return false;
		}
	}
	return true;
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
