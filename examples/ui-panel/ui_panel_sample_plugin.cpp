#include <D2RLPlugin/api.h>

static constexpr D2RL::PluginInfo UiPanelPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "ui-panel-sample",
	.name        = "UI Panel Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Registers and toggles a namespaced UI panel.",
	.flags       = D2RL::PluginFlags::Shared,
};

static constexpr char PanelLayout[] = R"json({
	"type": "Panel", "name": "ui-panel-sample/SamplePanel",
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
						"rect": { "x": 78, "y": 80, "width": 1076, "height": 70 },
						"text": "D2RLoader plugin panel",
						"style": "$StyleSettingsTitle"
					}
				},
				{
					"type": "TextBoxWidget", "name": "Help",
					"fields": {
						"rect": { "x": 78, "y": 165, "width": 1076, "height": 100 },
						"text": "This window uses a stock D2R sprite. Press Escape or run ui-panel-sample again.",
						"style": "$StyleModalDialogDescription"
					}
				}
			]
		}
	]
})json";

static const D2RL::PanelServiceV1*      panels;
static D2RL::Panels::RegistrationHandle panel = D2RL::Panels::InvalidHandle;

static auto TogglePanelCommand(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}
	return panels->togglePanel(command->plugin, panel) == D2RL::Panels::Result::Success ? D2RL::ConsoleCommandResult::Handled : D2RL::ConsoleCommandResult::Failed;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &UiPanelPluginInfo;
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
	const D2RL::Resources::ResourceRegistration resource {
		.structSize = D2RL::Resources::ResourceRegistrationSize,
		.path       = "data/global/ui/layouts/ui-panel-sample/SamplePanelhd.json",
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
	return context->RegisterConsoleCommand("ui-panel-sample", TogglePanelCommand, "Toggle the SDK sample panel.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
