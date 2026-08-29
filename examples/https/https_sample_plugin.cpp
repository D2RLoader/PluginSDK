#include <D2RLPlugin/api.h>
#include <cstdio>

static constexpr D2RL::PluginInfo HttpsPluginInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "https-sample",
	.name        = "HTTPS Sample Plugin",
	.version     = "0.1.0",
	.author      = "D2RLoader",
	.description = "Sends one asynchronous HTTPS GET request.",
	.flags       = D2RL::PluginFlags::Client,
};

static const D2RL::HttpServiceV1* http;

static void __cdecl OnResponse(const D2RL::PluginContext* context, const D2RL::Http::Response* response, void*) noexcept {
	if (context == nullptr || !D2RL::Http::HasResponseField(response, D2RL::Http::ResponseRequiredSize)) {
		return;
	}

	char message[256] {};
	std::snprintf(message, sizeof(message), "HTTPS request %llu finished: result=%u, status=%u, headers=%u, body=%u bytes.", static_cast<unsigned long long>(response->request), static_cast<uint32_t>(response->result), response->statusCode, response->headerCount, response->bodySize);
	context->LogInfo(message);
}

static auto SendRequest(D2R::Game::Client*, const D2RL::ConsoleCommandContext* command, void*) noexcept -> D2RL::ConsoleCommandResult {
	if (command == nullptr || command->plugin == nullptr || http == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	const D2RL::Http::Header headers[] {
		{ .name = "Accept", .value = "text/plain" },
	};
	const D2RL::Http::Request request {
		.structSize  = D2RL::Http::RequestSize,
		.method      = D2RL::Http::Method::Get,
		.url         = "https://example.com/",
		.headerCount = 1,
		.headers     = headers,
	};
	D2RL::Http::RequestHandle handle = D2RL::Http::InvalidRequestHandle;
	const D2RL::Http::Result  result = http->send(command->plugin, &request, OnResponse, nullptr, &handle);
	if (result != D2RL::Http::Result::Success) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	command->plugin->WriteConsoleMessage("HTTPS request queued. Its result will be written to the plugin log.");
	return D2RL::ConsoleCommandResult::Handled;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &HttpsPluginInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr || context->QueryService(D2RL::ServiceId::Http, D2RL::HttpServiceV1Version, &http) != D2RL::ServiceQueryResult::Success) {
		return false;
	}

	if (!D2RL::HasHttpServiceV1Field(http, D2RL::HttpServiceV1RequiredSize)) {
		return false;
	}

	return context->RegisterConsoleCommand("https-sample", SendRequest, "Send one asynchronous HTTPS GET request.");
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
