#include <D2RLPlugin/api.h>
#include <cstdio>
#include <cstring>

static constexpr D2RL::PluginInfo SharedEventsPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "shared-events-sample",
	.name        = "Shared Events Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Listens for item tooltips and one sample UI message.",
	.flags       = D2RL::PluginFlags::Client,
};

static const D2RL::WidgetServiceV1* widgets;

static void __cdecl OnItemTooltip(const D2RL::PluginContext*, D2RL::SharedEvents::ItemTooltipEvent* event, void* userData) noexcept {
	const char* text = static_cast<const char*>(userData);
	if (event == nullptr || event->structSize < D2RL::SharedEvents::ItemTooltipEventRequiredSize) {
		return;
	}

	if (event->text == nullptr || text == nullptr) {
		return;
	}
	const size_t size = std::strlen(text);
	if (size >= event->capacity) {
		return;
	}
	std::memcpy(event->text, text, size + 1);
	event->length = static_cast<uint32_t>(size);
}

static auto __cdecl OnUiMessage(const D2RL::PluginContext* context, const D2RL::SharedEvents::UiMessageEvent* event, void*) noexcept -> D2RL::SharedEvents::UiMessageAction {
	if (context == nullptr || event == nullptr) {
		return D2RL::SharedEvents::UiMessageAction::Continue;
	}

	if (event->structSize < D2RL::SharedEvents::UiMessageEventRequiredSize) {
		return D2RL::SharedEvents::UiMessageAction::Continue;
	}

	if (event->target == nullptr || event->command == nullptr) {
		return D2RL::SharedEvents::UiMessageAction::Continue;
	}
	if (event->characterHardcoreMode != D2RL::SharedEvents::CharacterHardcoreMode::Unknown) {
		const char* mode = event->characterHardcoreMode == D2RL::SharedEvents::CharacterHardcoreMode::Hardcore ? "Hardcore" : "Softcore";
		char        message[96] {};
		std::snprintf(message, sizeof(message), "Character creation requested in %s mode.", mode);
		context->LogInfo(message);
	}

	if (std::strcmp(event->target, "shared-events-sample") != 0 || std::strcmp(event->command, "ping") != 0) {
		return D2RL::SharedEvents::UiMessageAction::Continue;
	}
	constexpr char message[] = "The shared-events sample received its UI message.";
	context->WriteConsoleMessage(message);
	context->LogInfo(message);
	return D2RL::SharedEvents::UiMessageAction::Continue;
}

static auto SendSampleMessage(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	const D2RL::Widgets::UiAction action {
		.structSize = D2RL::Widgets::UiActionSize,
		.target     = "shared-events-sample",
		.command    = "ping",
		.text       = "hello",
	};
	return widgets->dispatchUiAction(command->plugin, &action) == D2RL::Widgets::Result::Success ? D2RL::ConsoleCommandResult::Handled : D2RL::ConsoleCommandResult::Failed;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &SharedEventsPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	const D2RL::SharedEventServiceV1* events = nullptr;
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::SharedEvent, D2RL::SharedEventServiceV1Version, &events) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasSharedEventServiceV1Field(events, D2RL::SharedEventServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Widget, D2RL::WidgetServiceV1Version, &widgets) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasWidgetServiceV1Field(widgets, D2RL::WidgetServiceV1RequiredSize)) {
		return false;
	}

	static char                                   AttributeText[] = "Plugin SDK line below Durability";
	const D2RL::SharedEvents::ItemTooltipListener attributeTooltip {
		.structSize = D2RL::SharedEvents::ItemTooltipListenerSize,
		.region     = D2RL::SharedEvents::ItemTooltipRegion::Attributes,
		.position   = D2RL::SharedEvents::ItemTooltipPosition::BelowAnchor,
		.anchor     = D2RL::SharedEvents::ItemTooltipAnchor::Durability,
		.fallback   = D2RL::SharedEvents::ItemTooltipFallback::RegionBottom,
		.callback   = OnItemTooltip,
		.userData   = AttributeText,
	};
	static char                                   ActionText[] = "Plugin SDK action-footer bottom";
	const D2RL::SharedEvents::ItemTooltipListener actionTooltip {
		.structSize = D2RL::SharedEvents::ItemTooltipListenerSize,
		.region     = D2RL::SharedEvents::ItemTooltipRegion::ActionFooter,
		.position   = D2RL::SharedEvents::ItemTooltipPosition::Bottom,
		.callback   = OnItemTooltip,
		.userData   = ActionText,
	};
	const D2RL::SharedEvents::UiMessageListener messages {
		.structSize = D2RL::SharedEvents::UiMessageListenerSize,
		.callback   = OnUiMessage,
	};
	D2RL::SharedEvents::ListenerHandle attributeHandle = D2RL::SharedEvents::InvalidHandle;
	D2RL::SharedEvents::ListenerHandle actionHandle    = D2RL::SharedEvents::InvalidHandle;
	D2RL::SharedEvents::ListenerHandle messageHandle   = D2RL::SharedEvents::InvalidHandle;
	if (events->registerItemTooltipListener(context, &attributeTooltip, &attributeHandle) != D2RL::SharedEvents::Result::Success) {
		return false;
	}

	if (events->registerItemTooltipListener(context, &actionTooltip, &actionHandle) != D2RL::SharedEvents::Result::Success) {
		return false;
	}

	if (events->registerUiMessageListener(context, &messages, &messageHandle) != D2RL::SharedEvents::Result::Success) {
		return false;
	}
	return context->RegisterConsoleCommand("shared-events-sample", SendSampleMessage, "Send a UI message through the shared-events sample.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
