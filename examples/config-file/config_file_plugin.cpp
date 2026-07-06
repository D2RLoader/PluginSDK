#include <D2RLPlugin/api.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>

static constexpr char DefaultConfig[] = R"toml(# Config File Sample Plugin

[config-file-sample]
enabled = true
message = "Hello from config-file-sample.toml"
)toml";

static constexpr D2RL::PluginInfo ConfigFilePluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "config-file-sample",
	.name        = "Config File Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Shows plugin-owned TOML config helpers.",
	.flags       = D2RL::PluginFlags::None,
};

static constexpr auto ByteSize(std::size_t size) noexcept -> std::uint32_t {
	return static_cast<std::uint32_t>(size);
}

static auto ConfigCommand(D2R::Game::Client* client, const D2RL::ConsoleCommandContext* command, void* userData) noexcept -> D2RL::ConsoleCommandResult {
	(void)client;
	(void)userData;

	if (command == nullptr || command->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	if (!command->plugin->EnsureConfig(DefaultConfig)) {
		command->plugin->WriteConsoleError("config-file-sample could not create its config file.");
		return D2RL::ConsoleCommandResult::Failed;
	}

	std::array<char, 4'096> config {};
	std::uint32_t           requiredSize = 0;
	if (!command->plugin->ReadConfig(config.data(), ByteSize(config.size()), &requiredSize)) {
		command->plugin->WriteConsoleError("config-file-sample could not read its config file.");
		return D2RL::ConsoleCommandResult::Failed;
	}

	char message[128] {};
	std::snprintf(message, sizeof(message), "config-file-sample.toml is available (%u bytes).", requiredSize);
	command->plugin->WriteConsoleMessage(message);
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &ConfigFilePluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	if (!context->EnsureConfig(DefaultConfig)) {
		context->LogError("config file sample could not create its config file.");
		return false;
	}

	context->LogInfo("config file sample loaded.");
	return context->RegisterConsoleCommand("config-file-sample", ConfigCommand, "Show config sample status.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
