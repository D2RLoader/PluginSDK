#pragma once

#include <cstddef>
#include <cstdint>

namespace D2RL::CoreExports {

inline constexpr const char* CoreDllName = "D2RCore.dll";

// Ordinal 0 is unused. D2RCore keeps 100-1999 for private loader calls.
inline constexpr uint16_t PublicOrdinalFirst  = 1;
inline constexpr uint16_t PublicOrdinalLast   = 99;
inline constexpr uint16_t PrivateOrdinalFirst = 100;
inline constexpr uint16_t PrivateOrdinalLast  = 1'999;

inline constexpr size_t MaxConsoleCommandLength = 512;

struct ExportInfo {
	const char* name;
	uint16_t    ordinal;
};

enum class CommandResult : int32_t {
	Handled          = 0,
	InvalidArguments = 1,
	Failed           = 2,
};

enum class CVarType : uint32_t {
	String = 0,
	Bool   = 1,
	Int    = 2,
	Float  = 3,
};

enum class MessageKind : int32_t {
	Input   = 0,
	Output  = 1,
	Debug   = 2,
	Error   = 3,
	Warning = 4,
};

enum class CommandScope : uint32_t {
	LocalOnly     = 0,
	RemoteAllowed = 1,
};

enum class CommandAccess : uint32_t {
	Any      = 0,
	Host     = 1,
	Operator = 2,
};

inline constexpr uint32_t CommandHidden       = 0x00000001U;
inline constexpr uint32_t SuppressCommandEcho = 0x00000002U;

struct CommandContext {
	uint32_t    structSize;
	uint32_t    inputLength;
	uint32_t    nameLength;
	uint32_t    argsLength;
	const char* input;
	const char* name;
	const char* args;
	void*       runtimeGame;
};

using CommandCallback = CommandResult(__fastcall*)(void* client, const CommandContext* command, void* userData);

struct CommandRegistration {
	uint32_t        structSize;
	uint32_t        flags;
	const char*     name;
	const char*     description;
	CommandCallback callback;
	void*           userData;
	const char*     usage;
	const char*     aliases;
	const char*     source;
	CommandScope    scope;
	CommandAccess   access;
};

struct CVarContext {
	uint32_t    structSize;
	uint32_t    inputLength;
	uint32_t    nameLength;
	uint32_t    argsLength;
	const char* input;
	const char* name;
	const char* args;
	const char* currentValue;
};

using CVarChangedCallback = CommandResult(__fastcall*)(const CVarContext* context, const char* value, void* userData);

struct CVarRegistration {
	uint32_t            structSize;
	CVarType            type;
	uint32_t            flags;
	const char*         name;
	const char*         description;
	const char*         defaultValue;
	CVarChangedCallback changedCallback;
	void*               userData;
};

struct TextBuffer {
	char*  data;
	size_t size;
};

struct WideTextBuffer {
	wchar_t* data;
	size_t   size;
};

struct BinCompileResult {
	uint32_t written;
	uint32_t skipped;
	uint32_t failed;
	uint8_t  ran;
	uint8_t  alreadyRunning;
	uint8_t  launchParamsMissing;
	uint8_t  noTargetMod;
};

struct PackMpqResult {
	uint32_t fileCount;
	uint32_t skippedExcelTxt;
	uint64_t sourceBytes;
	uint64_t outputBytes;
	uint8_t  listfile;
	uint8_t  reserved[7];
};

inline constexpr uint32_t PackMpqNoListfile      = 0x00000001U;
inline constexpr uint32_t PackMpqIncludeExcelTxt = 0x00000002U;

struct PackMpqRequest {
	const wchar_t* modName;
	uint32_t       flags;
	uint32_t       reserved;
	WideTextBuffer outputPath;
	TextBuffer     error;
};

using VersionFn               = const char*(__cdecl*)() noexcept;
using HasActiveModFn          = bool(__cdecl*)() noexcept;
using IsDebugModeEnabledFn    = bool(__cdecl*)() noexcept;
using RegisterCommandFn       = bool(__fastcall*)(const CommandRegistration* command) noexcept;
using RegisterCVarFn          = bool(__fastcall*)(const CVarRegistration* cvar) noexcept;
using WriteConsoleMessageFn   = void(__fastcall*)(const char* text, MessageKind messageKind) noexcept;
using GetModBuildVersionFn    = uint32_t(__cdecl*)() noexcept;
using ModIntegrationFn        = bool(__fastcall*)(const wchar_t* modName, char* errorBuffer, size_t errorBufferSize) noexcept;
using InitModIntegrationFn    = ModIntegrationFn;
using UpdateModIntegrationFn  = ModIntegrationFn;
using CompileModBinsFn        = bool(__fastcall*)(const wchar_t* modName, BinCompileResult* result, char* errorBuffer, size_t errorBufferSize) noexcept;
using PackModMpqFn            = bool(__fastcall*)(const PackMpqRequest* request, PackMpqResult* result) noexcept;
using IsInGameFn              = bool(__cdecl*)() noexcept;
using ExecuteConsoleCommandFn = bool(__fastcall*)(const char* command) noexcept;

inline constexpr ExportInfo VersionInfo { "Version", 1 };
inline constexpr ExportInfo HasActiveModInfo { "HasActiveMod", 2 };
inline constexpr ExportInfo IsDebugModeEnabledInfo { "IsDebugModeEnabled", 3 };
inline constexpr ExportInfo RegisterCommandInfo { "RegisterCommand", 4 };
inline constexpr ExportInfo RegisterCVarInfo { "RegisterCVar", 5 };
inline constexpr ExportInfo WriteConsoleMessageInfo { "WriteConsoleMessage", 6 };
inline constexpr ExportInfo GetModBuildVersionInfo { "GetModBuildVersion", 7 };
inline constexpr ExportInfo InitModIntegrationInfo { "InitModIntegration", 8 };
inline constexpr ExportInfo UpdateModIntegrationInfo { "UpdateModIntegration", 9 };
inline constexpr ExportInfo CompileModBinsInfo { "CompileModBins", 11 };
inline constexpr ExportInfo PackModMpqInfo { "PackModMpq", 12 };
inline constexpr ExportInfo IsInGameInfo { "IsInGame", 13 };
inline constexpr ExportInfo ExecuteConsoleCommandInfo { "ExecuteConsoleCommand", 14 };

static_assert(sizeof(CommandContext) == 48);
static_assert(sizeof(CommandRegistration) == 72);
static_assert(sizeof(CVarContext) == 48);
static_assert(sizeof(CVarRegistration) == 56);
static_assert(sizeof(BinCompileResult) == 16);
static_assert(sizeof(PackMpqResult) == 32);
static_assert(sizeof(PackMpqRequest) == 48);

}
