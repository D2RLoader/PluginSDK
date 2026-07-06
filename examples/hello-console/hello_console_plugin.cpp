#include <D2RLPlugin/api.h>

static constexpr D2RL::PluginInfo HelloPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "hello-console",
	.name        = "Hello Console Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Small D2RLoader SDK example.",
	.flags       = D2RL::PluginFlags::None,
};

static auto HelloCommand(D2R::Game::Client* client, const D2RL::ConsoleCommandContext* command, void* userData) noexcept -> D2RL::ConsoleCommandResult {
	(void)client;
	(void)userData;

	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	command->plugin->WriteConsoleMessage("hello from the D2RLoader SDK.");
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &HelloPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	context->LogInfo("hello console sample loaded.");
	return context->RegisterConsoleCommand("hello-plugin", HelloCommand, "Print a greeting from the sample plugin.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
