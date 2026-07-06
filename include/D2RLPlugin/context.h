#pragma once

#include <D2RLPlugin/console.h>
#include <D2RLPlugin/hooks.h>
#include <D2RLPlugin/lifecycle.h>
#include <D2RLPlugin/patching.h>
#include <cstdint>
#include <type_traits>

namespace D2RL {

struct PluginContext;

using LogFn           = void(__cdecl*)(const PluginContext* ctx, const char* message) noexcept;
using EnsureConfigFn  = bool(__cdecl*)(const PluginContext* ctx, const char* defaultToml) noexcept;
using ReadConfigFn    = bool(__cdecl*)(const PluginContext* ctx, char* output, uint32_t outputSize, uint32_t* requiredSize) noexcept;
using WriteConfigFn   = bool(__cdecl*)(const PluginContext* ctx, const char* tomlText) noexcept;
using GetPluginInfoFn = const PluginInfo*(__cdecl*)() noexcept;
using LoadPluginFn    = bool(__cdecl*)(const PluginContext* ctx) noexcept;
using UnloadPluginFn  = void(__cdecl*)() noexcept;

// apiSize lets plugins check which API fields are available.
struct PluginApi {
	uint32_t                 apiSize;
	uint32_t                 reserved0;
	LogFn                    logInfo;
	LogFn                    logWarn;
	LogFn                    logError;
	EnsureConfigFn           ensureConfig;
	ReadConfigFn             readConfig;
	WriteConfigFn            writeConfig;
	PatchBytesFn             patchBytes;
	PatchFillFn              patchFill;
	PatchWriteFn             patchWrite;
	PatchRel32Fn             patchRel32;
	uintptr_t                reserved[4];
	CheckExpectedBytesFn     checkExpectedBytes;
	uintptr_t                reserved2[4];
	RegisterConsoleCommandFn registerConsoleCommand;
	WriteConsoleMessageFn    writeConsoleMessage;
	InstallInlineHookFn      installInlineHook;
	uintptr_t                reserved3[4];
};

inline constexpr uint32_t PluginApiSize = static_cast<uint32_t>(sizeof(PluginApi));

inline auto HasApi(const PluginApi* api) noexcept -> bool {
	return api != nullptr && api->apiSize >= PluginApiSize;
}

inline auto ApiLogInfo(const PluginApi* api) noexcept -> LogFn {
	return HasApi(api) ? api->logInfo : nullptr;
}

inline auto ApiLogWarn(const PluginApi* api) noexcept -> LogFn {
	return HasApi(api) ? api->logWarn : nullptr;
}

inline auto ApiLogError(const PluginApi* api) noexcept -> LogFn {
	return HasApi(api) ? api->logError : nullptr;
}

inline auto ApiEnsureConfig(const PluginApi* api) noexcept -> EnsureConfigFn {
	return HasApi(api) ? api->ensureConfig : nullptr;
}

inline auto ApiReadConfig(const PluginApi* api) noexcept -> ReadConfigFn {
	return HasApi(api) ? api->readConfig : nullptr;
}

inline auto ApiWriteConfig(const PluginApi* api) noexcept -> WriteConfigFn {
	return HasApi(api) ? api->writeConfig : nullptr;
}

inline auto ApiPatchBytes(const PluginApi* api) noexcept -> PatchBytesFn {
	return HasApi(api) ? api->patchBytes : nullptr;
}

inline auto ApiPatchFill(const PluginApi* api) noexcept -> PatchFillFn {
	return HasApi(api) ? api->patchFill : nullptr;
}

inline auto ApiPatchWrite(const PluginApi* api) noexcept -> PatchWriteFn {
	return HasApi(api) ? api->patchWrite : nullptr;
}

inline auto ApiPatchRel32(const PluginApi* api) noexcept -> PatchRel32Fn {
	return HasApi(api) ? api->patchRel32 : nullptr;
}

inline auto ApiCheckExpectedBytes(const PluginApi* api) noexcept -> CheckExpectedBytesFn {
	return HasApi(api) ? api->checkExpectedBytes : nullptr;
}

inline auto ApiRegisterConsoleCommand(const PluginApi* api) noexcept -> RegisterConsoleCommandFn {
	return HasApi(api) ? api->registerConsoleCommand : nullptr;
}

inline auto ApiWriteConsoleMessage(const PluginApi* api) noexcept -> WriteConsoleMessageFn {
	return HasApi(api) ? api->writeConsoleMessage : nullptr;
}

inline auto ApiInstallInlineHook(const PluginApi* api) noexcept -> InstallInlineHookFn {
	return HasApi(api) ? api->installInlineHook : nullptr;
}

// Plugins receive this from D2RLoader during load. contextSize checks this
// data block after apiVersion has already been accepted.
struct PluginContext {
	uint32_t         contextSize;
	uint32_t         apiVersion;
	const PluginApi* api;
	LoadScope        loadScope;
	uint32_t         reserved0;
	uintptr_t        exeBase;
	const char*      pluginId;
	const char*      activeMod;
	const wchar_t*   modDirectory;
	const wchar_t*   pluginConfigPath;
	const wchar_t*   pluginLogPath;
	uintptr_t        reserved[4];
	const wchar_t*   scopeRootDirectory;
	const wchar_t*   pluginDirectory;
	const wchar_t*   pluginPath;
	const wchar_t*   modSupportDirectory;
	const char*      buildVersion;
	const char*      buildName;
	uint32_t         runtimeFlags;
	uint32_t         launchPlatform;
	uint32_t         modDataVersionBuild;
	uint32_t         reserved1;
	uintptr_t        reserved2[4];

	[[nodiscard]]
	auto GetApi() const noexcept -> const PluginApi* {
		const auto currentContextSize = static_cast<uint32_t>(sizeof(PluginContext));
		return contextSize >= currentContextSize && HasApi(api) ? api : nullptr;
	}

	void LogInfo(const char* message) const noexcept {
		const LogFn log = ApiLogInfo(GetApi());
		if (log != nullptr) {
			log(this, message);
		}
	}

	void LogWarn(const char* message) const noexcept {
		const LogFn log = ApiLogWarn(GetApi());
		if (log != nullptr) {
			log(this, message);
		}
	}

	void LogError(const char* message) const noexcept {
		const LogFn log = ApiLogError(GetApi());
		if (log != nullptr) {
			log(this, message);
		}
	}

	[[nodiscard]]
	auto PatchBytes(uint64_t rva, const void* expected, uint32_t expectedSize, const void* bytes, uint32_t size) const noexcept -> bool {
		const PatchBytesFn patch = ApiPatchBytes(GetApi());
		return patch != nullptr && patch(this, rva, expected, expectedSize, bytes, size);
	}

	[[nodiscard]]
	auto PatchFill(uint64_t rva, const void* expected, uint32_t expectedSize, uint8_t value, uint32_t size) const noexcept -> bool {
		const PatchFillFn patch = ApiPatchFill(GetApi());
		return patch != nullptr && patch(this, rva, expected, expectedSize, value, size);
	}

	[[nodiscard]]
	auto PatchNop(uint64_t rva, const void* expected, uint32_t expectedSize, uint32_t size) const noexcept -> bool {
		return PatchFill(rva, expected, expectedSize, 0x90, size);
	}

	[[nodiscard]]
	auto PatchWrite(uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t value, uint32_t size) const noexcept -> bool {
		const PatchWriteFn patch = ApiPatchWrite(GetApi());
		return patch != nullptr && patch(this, rva, expected, expectedSize, value, size);
	}

	[[nodiscard]]
	auto PatchWriteU8(uint64_t rva, const void* expected, uint32_t expectedSize, uint8_t value) const noexcept -> bool {
		return PatchWrite(rva, expected, expectedSize, value, sizeof(value));
	}

	[[nodiscard]]
	auto PatchWriteU16(uint64_t rva, const void* expected, uint32_t expectedSize, uint16_t value) const noexcept -> bool {
		return PatchWrite(rva, expected, expectedSize, value, sizeof(value));
	}

	[[nodiscard]]
	auto PatchWriteU32(uint64_t rva, const void* expected, uint32_t expectedSize, uint32_t value) const noexcept -> bool {
		return PatchWrite(rva, expected, expectedSize, value, sizeof(value));
	}

	[[nodiscard]]
	auto PatchWriteU64(uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t value) const noexcept -> bool {
		return PatchWrite(rva, expected, expectedSize, value, sizeof(value));
	}

	[[nodiscard]]
	auto PatchRel32(uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size, Rel32PatchKind kind) const noexcept -> bool {
		const PatchRel32Fn patch = ApiPatchRel32(GetApi());
		return patch != nullptr && patch(this, rva, expected, expectedSize, targetRva, size, kind);
	}

	[[nodiscard]]
	auto PatchJmpRel32(uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size = 5) const noexcept -> bool {
		return PatchRel32(rva, expected, expectedSize, targetRva, size, Rel32PatchKind::Jump);
	}

	[[nodiscard]]
	auto PatchCallRel32(uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size = 5) const noexcept -> bool {
		return PatchRel32(rva, expected, expectedSize, targetRva, size, Rel32PatchKind::Call);
	}

	[[nodiscard]]
	auto EnsureConfig(const char* defaultToml = "") const noexcept -> bool {
		const EnsureConfigFn ensure = ApiEnsureConfig(GetApi());
		return ensure != nullptr && ensure(this, defaultToml);
	}

	[[nodiscard]]
	auto ReadConfig(char* output, uint32_t outputSize, uint32_t* requiredSize = nullptr) const noexcept -> bool {
		const ReadConfigFn read = ApiReadConfig(GetApi());
		return read != nullptr && read(this, output, outputSize, requiredSize);
	}

	[[nodiscard]]
	auto WriteConfig(const char* tomlText) const noexcept -> bool {
		const WriteConfigFn write = ApiWriteConfig(GetApi());
		return write != nullptr && write(this, tomlText);
	}

	[[nodiscard]]
	auto CheckExpectedBytes(uint64_t rva, const void* expected, uint32_t expectedSize) const noexcept -> bool {
		const CheckExpectedBytesFn check = ApiCheckExpectedBytes(GetApi());
		return check != nullptr && check(this, rva, expected, expectedSize);
	}

	[[nodiscard]]
	auto RegisterConsoleCommand(ConsoleCommandRegistration command) const noexcept -> bool {
		if (command.registrationSize == 0) {
			command.registrationSize = ConsoleCommandRegistrationSize;
		}

		const RegisterConsoleCommandFn registerCommand = ApiRegisterConsoleCommand(GetApi());
		return registerCommand != nullptr && registerCommand(this, &command);
	}

	[[nodiscard]]
	auto RegisterConsoleCommand(const char* name, ConsoleCommandCallback callback, const char* description = nullptr, void* userData = nullptr) const noexcept -> bool {
		return RegisterConsoleCommand(MakeConsoleCommand(name, callback, description, userData));
	}

	void WriteConsoleMessage(const char* message, ConsoleMessageKind kind = ConsoleMessageKind::Output) const noexcept {
		const WriteConsoleMessageFn write = ApiWriteConsoleMessage(GetApi());
		if (write != nullptr) {
			write(this, message, kind);
		}
	}

	void WriteConsoleDebug(const char* message) const noexcept { WriteConsoleMessage(message, ConsoleMessageKind::Debug); }

	void WriteConsoleWarning(const char* message) const noexcept { WriteConsoleMessage(message, ConsoleMessageKind::Warning); }

	void WriteConsoleError(const char* message) const noexcept { WriteConsoleMessage(message, ConsoleMessageKind::Error); }

	[[nodiscard]]
	auto InstallInlineHook(InlineHookRegistration hook) const noexcept -> bool {
		if (hook.registrationSize == 0) {
			hook.registrationSize = InlineHookRegistrationSize;
		}

		const InstallInlineHookFn install = ApiInstallInlineHook(GetApi());
		return install != nullptr && install(this, &hook);
	}

	[[nodiscard]]
	auto InstallInlineHook(uint64_t rva, const void* expected, uint32_t expectedSize, void* target, void** original = nullptr) const noexcept -> bool {
		return InstallInlineHook(MakeInlineHook(rva, expected, expectedSize, target, original));
	}

	template <typename Function>
	[[nodiscard]]
	auto InstallInlineHook(uint64_t rva, const void* expected, uint32_t expectedSize, Function target, Function* original = nullptr) const noexcept -> bool {
		static_assert(std::is_pointer_v<Function>, "InstallInlineHook target must be a function pointer");
		static_assert(std::is_function_v<std::remove_pointer_t<Function>>, "InstallInlineHook target must be a function pointer");
		void*  originalAddress = nullptr;
		void** originalOutput  = original != nullptr ? &originalAddress : nullptr;
		void*  targetAddress   = reinterpret_cast<void*>(target);
		if (!InstallInlineHook(rva, expected, expectedSize, targetAddress, originalOutput)) {
			return false;
		}
		if (original != nullptr) {
			*original = reinterpret_cast<Function>(originalAddress);
		}
		return true;
	}
};

inline constexpr uint32_t PluginContextSize = static_cast<uint32_t>(sizeof(PluginContext));

inline auto HasContext(const PluginContext* ctx) noexcept -> bool {
	return ctx != nullptr && ctx->contextSize >= PluginContextSize;
}

inline auto GetApi(const PluginContext* ctx) noexcept -> const PluginApi* {
	return HasContext(ctx) ? ctx->GetApi() : nullptr;
}

inline auto PatchBytes(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, const void* bytes, uint32_t size) noexcept -> bool {
	return ctx != nullptr && ctx->PatchBytes(rva, expected, expectedSize, bytes, size);
}

inline auto PatchFill(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint8_t value, uint32_t size) noexcept -> bool {
	return ctx != nullptr && ctx->PatchFill(rva, expected, expectedSize, value, size);
}

inline auto PatchNop(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint32_t size) noexcept -> bool {
	return ctx != nullptr && ctx->PatchNop(rva, expected, expectedSize, size);
}

inline auto PatchWrite(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t value, uint32_t size) noexcept -> bool {
	return ctx != nullptr && ctx->PatchWrite(rva, expected, expectedSize, value, size);
}

inline auto PatchWriteU8(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint8_t value) noexcept -> bool {
	return ctx != nullptr && ctx->PatchWriteU8(rva, expected, expectedSize, value);
}

inline auto PatchWriteU16(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint16_t value) noexcept -> bool {
	return ctx != nullptr && ctx->PatchWriteU16(rva, expected, expectedSize, value);
}

inline auto PatchWriteU32(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint32_t value) noexcept -> bool {
	return ctx != nullptr && ctx->PatchWriteU32(rva, expected, expectedSize, value);
}

inline auto PatchWriteU64(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t value) noexcept -> bool {
	return ctx != nullptr && ctx->PatchWriteU64(rva, expected, expectedSize, value);
}

inline auto PatchRel32(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size, Rel32PatchKind kind) noexcept -> bool {
	return ctx != nullptr && ctx->PatchRel32(rva, expected, expectedSize, targetRva, size, kind);
}

inline auto PatchJmpRel32(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size = 5) noexcept -> bool {
	return ctx != nullptr && ctx->PatchJmpRel32(rva, expected, expectedSize, targetRva, size);
}

inline auto PatchCallRel32(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size = 5) noexcept -> bool {
	return ctx != nullptr && ctx->PatchCallRel32(rva, expected, expectedSize, targetRva, size);
}

inline auto EnsureConfig(const PluginContext* ctx, const char* defaultToml = "") noexcept -> bool {
	return ctx != nullptr && ctx->EnsureConfig(defaultToml);
}

inline auto ReadConfig(const PluginContext* ctx, char* output, uint32_t outputSize, uint32_t* requiredSize = nullptr) noexcept -> bool {
	return ctx != nullptr && ctx->ReadConfig(output, outputSize, requiredSize);
}

inline auto WriteConfig(const PluginContext* ctx, const char* tomlText) noexcept -> bool {
	return ctx != nullptr && ctx->WriteConfig(tomlText);
}

inline auto CheckExpectedBytes(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize) noexcept -> bool {
	return ctx != nullptr && ctx->CheckExpectedBytes(rva, expected, expectedSize);
}

inline auto RegisterConsoleCommand(const PluginContext* ctx, ConsoleCommandRegistration command) noexcept -> bool {
	return ctx != nullptr && ctx->RegisterConsoleCommand(command);
}

inline auto RegisterConsoleCommand(const PluginContext* ctx, const char* name, ConsoleCommandCallback callback, const char* description = nullptr, void* userData = nullptr) noexcept -> bool {
	return ctx != nullptr && ctx->RegisterConsoleCommand(name, callback, description, userData);
}

inline void WriteConsoleMessage(const PluginContext* ctx, const char* message, ConsoleMessageKind kind = ConsoleMessageKind::Output) noexcept {
	if (ctx != nullptr) {
		ctx->WriteConsoleMessage(message, kind);
	}
}

inline void WriteConsoleDebug(const PluginContext* ctx, const char* message) noexcept {
	WriteConsoleMessage(ctx, message, ConsoleMessageKind::Debug);
}

inline void WriteConsoleWarning(const PluginContext* ctx, const char* message) noexcept {
	WriteConsoleMessage(ctx, message, ConsoleMessageKind::Warning);
}

inline void WriteConsoleError(const PluginContext* ctx, const char* message) noexcept {
	WriteConsoleMessage(ctx, message, ConsoleMessageKind::Error);
}

inline auto InstallInlineHook(const PluginContext* ctx, InlineHookRegistration hook) noexcept -> bool {
	return ctx != nullptr && ctx->InstallInlineHook(hook);
}

inline auto InstallInlineHook(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, void* target, void** original = nullptr) noexcept -> bool {
	return ctx != nullptr && ctx->InstallInlineHook(rva, expected, expectedSize, target, original);
}

template <typename Function>
inline auto InstallInlineHook(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, Function target, Function* original = nullptr) noexcept -> bool {
	return ctx != nullptr && ctx->InstallInlineHook(rva, expected, expectedSize, target, original);
}

inline auto GetScopeRootDirectory(const PluginContext* ctx) noexcept -> const wchar_t* {
	return HasContext(ctx) ? ctx->scopeRootDirectory : nullptr;
}

inline auto GetPluginDirectory(const PluginContext* ctx) noexcept -> const wchar_t* {
	return HasContext(ctx) ? ctx->pluginDirectory : nullptr;
}

inline auto GetPluginPath(const PluginContext* ctx) noexcept -> const wchar_t* {
	return HasContext(ctx) ? ctx->pluginPath : nullptr;
}

inline auto GetModSupportDirectory(const PluginContext* ctx) noexcept -> const wchar_t* {
	return HasContext(ctx) ? ctx->modSupportDirectory : nullptr;
}

inline auto GetBuildVersion(const PluginContext* ctx) noexcept -> const char* {
	return HasContext(ctx) ? ctx->buildVersion : nullptr;
}

inline auto GetBuildName(const PluginContext* ctx) noexcept -> const char* {
	return HasContext(ctx) ? ctx->buildName : nullptr;
}

inline auto GetRuntimeFlags(const PluginContext* ctx) noexcept -> uint32_t {
	return HasContext(ctx) ? ctx->runtimeFlags : 0;
}

inline auto GetLaunchPlatform(const PluginContext* ctx) noexcept -> uint32_t {
	return HasContext(ctx) ? ctx->launchPlatform : 0;
}

inline auto GetModDataVersionBuild(const PluginContext* ctx) noexcept -> uint32_t {
	return HasContext(ctx) ? ctx->modDataVersionBuild : 0;
}

}
