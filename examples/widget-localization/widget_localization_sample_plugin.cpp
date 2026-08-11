#include <D2RLPlugin/api.h>
#include <array>
#include <cstdio>

static constexpr D2RL::PluginInfo WidgetLocalizationPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "widget-localization-sample",
	.name        = "Widget and Localization Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Resolves localized text and inspects a namespaced widget.",
	.flags       = D2RL::PluginFlags::Client,
};

static constexpr char PanelLayout[] = R"json({
	"type": "Panel", "name": "widget-localization-sample/SamplePanel",
	"fields": {
		"anchor": { "x": 0.5, "y": 0.5 },
		"priority": 8500
	},
	"children": [
		{
			"type": "ImageWidget", "name": "Frame",
			"fields": {
				"rect": { "x": -627, "y": -270 },
				"filename": "Panel\\Modals\\Modal_No_Title_BG"
			},
			"children": [
				{
					"type": "TextBoxWidget", "name": "Heading",
					"fields": {
						"rect": { "x": 78, "y": 70, "width": 1076, "height": 70 },
						"text": "Widget and localization services",
						"style": "$StyleSettingsTitle"
					}
				},
				{
					"type": "TextBoxWidget", "name": "Help",
					"fields": {
						"rect": { "x": 150, "y": 150, "width": 956, "height": 130 },
						"text": "The command finds the named button below, reads its rectangle, and resolves its label through D2R's localization table.",
						"style": "$StyleModalDialogDescription"
					}
				},
				{
					"type": "ButtonWidget", "name": "LocalizedCancel",
					"fields": {
						"rect": { "x": 477, "y": 330 },
						"filename": "Panel\\Modals\\ModalButton",
						"focusIndicatorFilename": "Controller/HoverImages/ModalButton_Hover",
						"pressedFrame": 1,
						"disabledFrame": 2,
						"hoveredFrame": 3,
						"textString": "@strCancel",
						"pointSize": "$MediumFontSize",
						"onClickMessage": "PanelManager:ClosePanel:widget-localization-sample/SamplePanel",
						"textColor": "$FontColorWhite"
					}
				}
			]
		}
	]
})json";

static const D2RL::LocalizationServiceV1* localization;
static const D2RL::PanelServiceV1*        panels;
static const D2RL::WidgetServiceV1*       widgets;
static D2RL::Panels::RegistrationHandle   panel = D2RL::Panels::InvalidHandle;

static auto WidgetLocalizationCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (panels->openPanel(command->plugin, panel) != D2RL::Panels::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	uint32_t required = 0;
	if (localization->getStringByKey(command->plugin, "strCancel", nullptr, 0, &required) != D2RL::Localization::Result::BufferTooSmall) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	std::array<char, 128> text {};
	if (required == 0 || required > text.size()) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (localization->getStringByKey(command->plugin, "strCancel", text.data(), static_cast<uint32_t>(text.size()), &required) != D2RL::Localization::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	D2RL::Widgets::WidgetHandle panelWidget  = D2RL::Widgets::InvalidHandle;
	D2RL::Widgets::WidgetHandle cancelWidget = D2RL::Widgets::InvalidHandle;
	D2RL::Widgets::Rect         rect {};
	if (widgets->findPanel(command->plugin, "widget-localization-sample/SamplePanel", &panelWidget) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->findWidget(command->plugin, panelWidget, "LocalizedCancel", &cancelWidget) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->getWidgetRect(command->plugin, cancelWidget, &rect) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->setWidgetVisible(command->plugin, cancelWidget, true) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->setWidgetEnabled(command->plugin, cancelWidget, true) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	char message[224] {};
	std::snprintf(message, sizeof(message), "Localized strCancel=\"%s\"; LocalizedCancel rect is %d,%d %dx%d.", text.data(), rect.x, rect.y, rect.width, rect.height);
	command->plugin->WriteConsoleMessage(message);
	command->plugin->LogInfo(message);
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &WidgetLocalizationPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	const D2RL::ResourceServiceV1* resources = nullptr;
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Resource, D2RL::ResourceServiceV1Version, &resources) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasResourceServiceV1Field(resources, D2RL::ResourceServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Panel, D2RL::PanelServiceV1Version, &panels) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasPanelServiceV1Field(panels, D2RL::PanelServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Widget, D2RL::WidgetServiceV1Version, &widgets) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasWidgetServiceV1Field(widgets, D2RL::WidgetServiceV1RequiredSize)) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Localization, D2RL::LocalizationServiceV1Version, &localization) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasLocalizationServiceV1Field(localization, D2RL::LocalizationServiceV1RequiredSize)) {
		return false;
	}

	const D2RL::Resources::ResourceRegistration resource {
		.structSize = D2RL::Resources::ResourceRegistrationSize,
		.path       = "data/global/ui/layouts/widget-localization-sample/SamplePanelhd.json",
		.bytes      = PanelLayout,
		.byteCount  = sizeof(PanelLayout) - 1,
	};
	D2RL::Resources::RegistrationHandle resourceHandle = D2RL::Resources::InvalidHandle;
	if (resources->registerResource(context, &resource, &resourceHandle) != D2RL::Resources::Result::Success) {
		return false;
	}
	const D2RL::Panels::PanelRegistration registration {
		.structSize = D2RL::Panels::PanelRegistrationSize,
		.flags      = D2RL::Panels::PanelFlags::CloseOnEscape,
		.localId    = "SamplePanel",
	};
	if (panels->registerPanel(context, &registration, &panel) != D2RL::Panels::Result::Success) {
		return false;
	}
	return context->RegisterConsoleCommand("widget-localization-sample", WidgetLocalizationCommand, "Open a panel and report localized text and widget geometry.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
