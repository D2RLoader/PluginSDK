#pragma once

#include <D2RLPlugin/context.h>
#include <cstdarg>
#include <cstdio>

namespace D2RL {

inline void LogInfo(const PluginContext* ctx, const char* message) noexcept {
	if (ctx != nullptr) {
		ctx->LogInfo(message);
	}
}

inline void LogWarn(const PluginContext* ctx, const char* message) noexcept {
	if (ctx != nullptr) {
		ctx->LogWarn(message);
	}
}

inline void LogError(const PluginContext* ctx, const char* message) noexcept {
	if (ctx != nullptr) {
		ctx->LogError(message);
	}
}

inline void LogV(const PluginContext* ctx, LogFn log, const char* format, va_list args) noexcept {
	if (log == nullptr || format == nullptr) {
		return;
	}

	char message[1'024] {};
	std::vsnprintf(message, sizeof(message), format, args);
	log(ctx, message);
}

inline void LogInfoF(const PluginContext* ctx, const char* format, ...) noexcept {
	va_list args;
	va_start(args, format);
	const PluginApi* api = GetApi(ctx);
	LogV(ctx, ApiLogInfo(api), format, args);
	va_end(args);
}

inline void LogWarnF(const PluginContext* ctx, const char* format, ...) noexcept {
	va_list args;
	va_start(args, format);
	const PluginApi* api = GetApi(ctx);
	LogV(ctx, ApiLogWarn(api), format, args);
	va_end(args);
}

inline void LogErrorF(const PluginContext* ctx, const char* format, ...) noexcept {
	va_list args;
	va_start(args, format);
	const PluginApi* api = GetApi(ctx);
	LogV(ctx, ApiLogError(api), format, args);
	va_end(args);
}

}
