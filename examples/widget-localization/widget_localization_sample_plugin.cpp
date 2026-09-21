#include <D2RLPlugin/api.h>
#include <array>
#include <cstdio>

static constexpr D2RL::PluginInfo WidgetLocalizationPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.abiVersion  = D2RL_PLUGIN_ABI_VERSION,
	.id          = "widget-localization-sample",
	.name        = "Widget and Localization Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Reads input text, resolves localized text, and inspects a namespaced widget.",
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
						"text": "Type below, then run widget-localization-sample again. The command reads the input text, button rectangle, and localized label.",
						"style": "$StyleModalDialogDescription"
					}
				},
				{
					"type": "InputTextBoxWidget", "name": "SampleInput",
					"fields": {
						"rect": { "x": 150, "y": 275, "width": 956, "height": 50 },
						"maxStringLength": 127,
						"canInsertNewlines": false,
						"backgroundColor": "$FontColorTransparent",
						"fontType": "12ptF",
						"fontStyle": {
							"fontFace": "BlizzardGlobal",
							"fontColor": "$FontColorWhite",
							"pointSize": "$SmallFontSize"
						},
						"hint": {
							"text": "Type text here",
							"style": "$StyleModalDialogDescription"
						}
					}
				},
				{
					"type": "ButtonWidget", "name": "LocalizedCancel",
					"fields": {
						"rect": { "x": 477, "y": 380 },
						"filename": "Panel\\Modals\\ModalButton",
						"focusIndicatorFilename": "Controller/HoverImages/ModalButton_Hover",
						"pressedFrame": 1,
						"disabledFrame": 2,
						"hoveredFrame": 3,
						"textString": "@d2r:strCancel",
						"pointSize": "$MediumFontSize",
						"onClickMessage": "PanelManager:ClosePanel:widget-localization-sample/SamplePanel",
						"textColor": "$FontColorWhite"
					}
				}
			]
		}
	]
})json";

static const D2RL::LocalizationService* localization;
static const D2RL::PanelService*        panels;
static const D2RL::WidgetService*       widgets;
static D2RL::Panels::RegistrationHandle panel = D2RL::Panels::InvalidHandle;

static auto WidgetLocalizationCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (panels->openPanel(command->plugin, panel) != D2RL::Panels::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	uint32_t required = 0;
	if (localization->getStringByKey(command->plugin, "d2r:strCancel", nullptr, 0, &required) != D2RL::Localization::Result::BufferTooSmall) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	std::array<char, 128> text {};
	if (required == 0 || required > text.size()) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (localization->getStringByKey(command->plugin, "d2r:strCancel", text.data(), static_cast<uint32_t>(text.size()), &required) != D2RL::Localization::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	D2RL::Widgets::WidgetHandle panelWidget  = D2RL::Widgets::InvalidHandle;
	D2RL::Widgets::WidgetHandle cancelWidget = D2RL::Widgets::InvalidHandle;
	D2RL::Widgets::WidgetHandle inputWidget  = D2RL::Widgets::InvalidHandle;
	D2RL::Widgets::Rect         rect {};
	if (widgets->findPanel(command->plugin, "widget-localization-sample/SamplePanel", &panelWidget) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->findWidget(command->plugin, panelWidget, "LocalizedCancel", &cancelWidget) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->findWidget(command->plugin, panelWidget, "SampleInput", &inputWidget) != D2RL::Widgets::Result::Success) {
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

	uint32_t inputRequired = 0;
	if (widgets->getInputText(command->plugin, inputWidget, nullptr, 0, &inputRequired) != D2RL::Widgets::Result::BufferTooSmall) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	std::array<char, 128> inputText {};
	if (inputRequired == 0 || inputRequired > inputText.size()) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (widgets->getInputText(command->plugin, inputWidget, inputText.data(), static_cast<uint32_t>(inputText.size()), &inputRequired) != D2RL::Widgets::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	char message[384] {};
	std::snprintf(message, sizeof(message), "Input=\"%s\"; localized strCancel=\"%s\"; LocalizedCancel rect is %d,%d %dx%d.", inputText.data(), text.data(), rect.x, rect.y, rect.width, rect.height);
	command->plugin->WriteConsoleMessage(message);
	command->plugin->LogInfo(message);
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &WidgetLocalizationPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	const D2RL::ResourceService* resources = nullptr;
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(&resources) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasResourceServiceField(resources, D2RL::ResourceServiceRequiredSize)) {
		return false;
	}

	if (context->QueryService(&panels) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasPanelServiceField(panels, D2RL::PanelServiceRequiredSize)) {
		return false;
	}

	if (context->QueryService(&widgets) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasWidgetServiceField(widgets, D2RL::WidgetServiceRequiredSize)) {
		return false;
	}

	if (context->QueryService(&localization) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasLocalizationServiceField(localization, D2RL::LocalizationServiceRequiredSize)) {
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
	return context->RegisterConsoleCommand("widget-localization-sample", WidgetLocalizationCommand, "Open a panel and report its input text, localized text, and widget geometry.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
