#include <D2RLPlugin/api.h>

static constexpr D2RL::PluginInfo InputActionPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "input-action-sample",
	.name        = "Input Action Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Registers one bindable action in the Controls menu.",
	.flags       = D2RL::PluginFlags::Client,
};

static auto __cdecl OnAction(const D2RL::PluginContext* context, const D2RL::Input::ActionEvent* event, void*) noexcept -> D2RL::Input::ActionResult {
	if (context == nullptr || !D2RL::Input::HasActionEventField(event, D2RL::Input::ActionEventRequiredSize) || event->kind != D2RL::Input::ActionEventKind::Pressed) {
		return D2RL::Input::ActionResult::Ignored;
	}
	context->LogInfo("The Plugin SDK sample action was pressed.");
	return D2RL::Input::ActionResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &InputActionPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	const D2RL::InputServiceV1* input = nullptr;
	if (context == nullptr) {
		return false;
	}

	if (context->QueryService(D2RL::ServiceId::Input, D2RL::InputServiceV1Version, &input) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasInputServiceV1Field(input, D2RL::InputServiceV1RequiredSize)) {
		return false;
	}

	const D2RL::Input::ActionRegistration registration {
		.structSize       = D2RL::Input::ActionRegistrationSize,
		.logicalId        = "sample-action",
		.displayName      = "Plugin SDK Sample Action",
		.category         = "D2RLoader Plugin SDK",
		.defaultPrimary   = { .key = D2RL::Input::Key::F8 },
		.defaultSecondary = { .key = D2RL::Input::Key::None },
		.callback         = OnAction,
	};
	D2RL::Input::ActionHandle action = D2RL::Input::InvalidHandle;
	return input->registerAction(context, &registration, &action) == D2RL::Input::Result::Success && action != D2RL::Input::InvalidHandle;
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
