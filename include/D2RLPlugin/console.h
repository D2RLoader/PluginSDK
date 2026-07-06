#pragma once

#include <cstddef>
#include <cstdint>

namespace D2R::Game {

struct Client;

}

namespace D2RL {

struct PluginContext;

enum class ConsoleCommandResult : uint32_t {
	Handled          = 0,
	InvalidArguments = 1,
	Failed           = 2,
};

enum class ConsoleMessageKind : uint32_t {
	Output  = 1,
	Debug   = 2,
	Error   = 3,
	Warning = 4,
};

enum class ConsoleCommandScope : uint32_t {
	LocalOnly     = 0,
	RemoteAllowed = 1,
};

enum class ConsoleCommandAccess : uint32_t {
	Any      = 0,
	Host     = 1,
	Operator = 2,
};

inline constexpr uint32_t ConsoleCommandHidden       = 0x00000001U;
inline constexpr uint32_t ConsoleCommandSuppressEcho = 0x00000002U;

struct ConsoleCommandContext {
	uint32_t             contextSize;
	uint32_t             inputLength;
	uint32_t             nameLength;
	uint32_t             argsLength;
	const char*          input;
	const char*          name;
	const char*          args;
	const PluginContext* plugin;
	uint32_t             reserved[4];
};

using ConsoleCommandCallback = ConsoleCommandResult(__cdecl*)(D2R::Game::Client* client, const ConsoleCommandContext* command, void* userData) noexcept;

struct ConsoleCommandRegistration {
	uint32_t               registrationSize;
	uint32_t               flags;
	const char*            name;
	const char*            description;
	ConsoleCommandCallback callback;
	void*                  userData;
	const char*            usage;
	const char*            aliases;
	ConsoleCommandScope    scope;
	ConsoleCommandAccess   access;
	uintptr_t              reserved[3];
};

inline constexpr uint32_t ConsoleCommandContextSize              = static_cast<uint32_t>(sizeof(ConsoleCommandContext));
inline constexpr uint32_t ConsoleCommandRegistrationSize         = static_cast<uint32_t>(sizeof(ConsoleCommandRegistration));
inline constexpr uint32_t ConsoleCommandRegistrationRequiredSize = static_cast<uint32_t>(offsetof(ConsoleCommandRegistration, userData) + sizeof(void*));
inline constexpr uint32_t ConsoleCommandRegistrationUsageSize    = static_cast<uint32_t>(offsetof(ConsoleCommandRegistration, usage) + sizeof(const char*));
inline constexpr uint32_t ConsoleCommandRegistrationAliasesSize  = static_cast<uint32_t>(offsetof(ConsoleCommandRegistration, aliases) + sizeof(const char*));
inline constexpr uint32_t ConsoleCommandRegistrationScopeSize    = static_cast<uint32_t>(offsetof(ConsoleCommandRegistration, scope) + sizeof(ConsoleCommandScope));
inline constexpr uint32_t ConsoleCommandRegistrationAccessSize   = static_cast<uint32_t>(offsetof(ConsoleCommandRegistration, access) + sizeof(ConsoleCommandAccess));

inline auto HasConsoleCommandRegistrationField(const ConsoleCommandRegistration* command, uint32_t fieldEndOffset) noexcept -> bool {
	return command != nullptr && command->registrationSize >= fieldEndOffset;
}

inline auto ConsoleCommandUsage(const ConsoleCommandRegistration* command) noexcept -> const char* {
	return HasConsoleCommandRegistrationField(command, ConsoleCommandRegistrationUsageSize) ? command->usage : nullptr;
}

inline auto ConsoleCommandAliases(const ConsoleCommandRegistration* command) noexcept -> const char* {
	return HasConsoleCommandRegistrationField(command, ConsoleCommandRegistrationAliasesSize) ? command->aliases : nullptr;
}

inline auto ConsoleCommandScopeValue(const ConsoleCommandRegistration* command) noexcept -> ConsoleCommandScope {
	return HasConsoleCommandRegistrationField(command, ConsoleCommandRegistrationScopeSize) ? command->scope : ConsoleCommandScope::LocalOnly;
}

inline auto ConsoleCommandAccessValue(const ConsoleCommandRegistration* command) noexcept -> ConsoleCommandAccess {
	return HasConsoleCommandRegistrationField(command, ConsoleCommandRegistrationAccessSize) ? command->access : ConsoleCommandAccess::Any;
}

inline auto MakeConsoleCommand(const char* name, ConsoleCommandCallback callback, const char* description = nullptr, void* userData = nullptr) noexcept -> ConsoleCommandRegistration {
	return {
		.registrationSize = ConsoleCommandRegistrationSize,
		.name             = name,
		.description      = description,
		.callback         = callback,
		.userData         = userData,
	};
}

using RegisterConsoleCommandFn = bool(__cdecl*)(const PluginContext* ctx, const ConsoleCommandRegistration* command) noexcept;
using WriteConsoleMessageFn    = void(__cdecl*)(const PluginContext* ctx, const char* message, ConsoleMessageKind kind) noexcept;

}
