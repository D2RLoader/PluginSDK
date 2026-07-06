#include <D2RLPlugin/api.h>
#include <cstddef>
#include <cstdint>

// These memory patch examples write the bytes already present in the image.
static constexpr bool ApplyMemoryPatchExamples = true;

// Native hooks redirect game code into plugin code. Keep this false unless you are testing hooks.
// PatchingSampleFlags below adds D2RL::PluginFlags::NativeHooks when this is enabled.
static constexpr bool InstallNativeHookExample = false;

static constexpr D2RL::PluginFlags PatchingSampleFlags = InstallNativeHookExample ? D2RL::PluginFlags::NativeHooks : D2RL::PluginFlags::None;

using GetVisibleLineCountFn = std::uint32_t(__fastcall*)() noexcept;
using MemoryPatchExampleFn  = bool (*)(const D2RL::PluginContext*) noexcept;

static GetVisibleLineCountFn OriginalGetVisibleLineCount = nullptr;

struct ExpectedPatchRange {
	std::uint64_t rva;
	const void*   bytes;
	std::uint32_t size;
};

struct MemoryPatchExample {
	const char*          failureMessage;
	MemoryPatchExampleFn apply;
};

static constexpr auto ByteSize(std::size_t size) noexcept -> std::uint32_t {
	return static_cast<std::uint32_t>(size);
}

template <std::size_t Size>
static constexpr auto ByteCount(const std::uint8_t (&)[Size]) noexcept -> std::uint32_t {
	return ByteSize(Size);
}

template <std::size_t Size>
static constexpr auto MakeExpectedRange(std::uint64_t rva, const std::uint8_t (&bytes)[Size]) noexcept -> ExpectedPatchRange {
	return { rva, bytes, ByteCount(bytes) };
}

static constexpr std::uint8_t ExpectedBytesPatch[] { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
static constexpr std::uint8_t ReplacementBytesPatch[] { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };
static constexpr std::uint8_t ExpectedFillPatch[] { 0xCC, 0xCC, 0xCC, 0xCC };
static constexpr std::uint8_t ExpectedNopPatch[] { 0x90, 0x90, 0x90, 0x90, 0x90 };
static constexpr std::uint8_t ExpectedU8Patch[] { 0xCC };
static constexpr std::uint8_t ExpectedU16Patch[] { 0xCC, 0xCC };
static constexpr std::uint8_t ExpectedU32Patch[] { 0xCC, 0xCC, 0xCC, 0xCC };
static constexpr std::uint8_t ExpectedU64Patch[] { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
static constexpr std::uint8_t ExpectedJmpPatch[] { 0xE9, 0x83, 0x01, 0x00, 0x00 };
static constexpr std::uint8_t ExpectedCallPatch[] { 0xE8, 0x70, 0xB1, 0x04, 0x00 };

static constexpr ExpectedPatchRange ExpectedPatchRanges[] {
	MakeExpectedRange(0x00003601, ExpectedBytesPatch),
	MakeExpectedRange(0x00003454, ExpectedFillPatch),
	MakeExpectedRange(0x0041088A, ExpectedNopPatch),
	MakeExpectedRange(0x00003963, ExpectedU8Patch),
	MakeExpectedRange(0x00003964, ExpectedU16Patch),
	MakeExpectedRange(0x00003966, ExpectedU32Patch),
	MakeExpectedRange(0x0065118E, ExpectedU64Patch),
	MakeExpectedRange(0x00002716, ExpectedJmpPatch),
	MakeExpectedRange(0x0002EB5B, ExpectedCallPatch),
};

static constexpr std::uint64_t GetVisibleLineCountRva = 0x000A8950;
static constexpr std::uint8_t  ExpectedGetVisibleLineCount[] { 0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20 };

static auto __fastcall HookGetVisibleLineCount() noexcept -> std::uint32_t {
	const GetVisibleLineCountFn original = OriginalGetVisibleLineCount;
	return original != nullptr ? original() : 0U;
}

static constexpr D2RL::PluginInfo PatchingSampleInfo {
	.infoSize    = D2RL::PluginInfoSize,
	.apiVersion  = D2RL_PLUGIN_API_VERSION,
	.id          = "patching-sample",
	.name        = "Patching Sample Plugin",
	.version     = "0.1.0",
	.author      = "Testificate",
	.description = "Shows D2RLoader memory patches and inline hooks.",
	.flags       = PatchingSampleFlags,
};

static auto PatchingSampleCommand(D2R::Game::Client* client, const D2RL::ConsoleCommandContext* cmd, void* userData) noexcept -> D2RL::ConsoleCommandResult {
	(void)client;
	(void)userData;

	if (cmd == nullptr || cmd->plugin == nullptr) {
		return D2RL::ConsoleCommandResult::Failed;
	}

	cmd->plugin->WriteConsoleMessage("patching sample plugin is loaded.");
	cmd->plugin->WriteConsoleMessage(ApplyMemoryPatchExamples ? "memory patch examples are enabled." : "memory patch examples are disabled in source.");
	cmd->plugin->WriteConsoleMessage(InstallNativeHookExample ? "native hook example is enabled." : "native hook example is disabled in source.");
	return D2RL::ConsoleCommandResult::Handled;
}

static auto VerifyPatchInputs(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	for (const ExpectedPatchRange& range : ExpectedPatchRanges) {
		if (!context->CheckExpectedBytes(range.rva, range.bytes, range.size)) {
			return false;
		}
	}

	return true;
}

static auto ApplyBytesPatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedBytesPatch);
	const std::uint32_t bytesSize    = ByteCount(ReplacementBytesPatch);
	return context->PatchBytes(0x00003601, ExpectedBytesPatch, expectedSize, ReplacementBytesPatch, bytesSize);
}

static auto ApplyFillPatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedFillPatch);
	return context->PatchFill(0x00003454, ExpectedFillPatch, expectedSize, 0xCC, expectedSize);
}

static auto ApplyNopPatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedNopPatch);
	return context->PatchNop(0x0041088A, ExpectedNopPatch, expectedSize, expectedSize);
}

static auto ApplyWriteU8PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedU8Patch);
	return context->PatchWriteU8(0x00003963, ExpectedU8Patch, expectedSize, 0xCC);
}

static auto ApplyWriteU16PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedU16Patch);
	return context->PatchWriteU16(0x00003964, ExpectedU16Patch, expectedSize, 0xCCCC);
}

static auto ApplyWriteU32PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedU32Patch);
	return context->PatchWriteU32(0x00003966, ExpectedU32Patch, expectedSize, 0xCCCCCCCC);
}

static auto ApplyWriteU64PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedU64Patch);
	return context->PatchWriteU64(0x0065118E, ExpectedU64Patch, expectedSize, 0x9090909090909090ULL);
}

static auto ApplyJmpRel32PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedJmpPatch);
	return context->PatchJmpRel32(0x00002716, ExpectedJmpPatch, expectedSize, 0x0000289E, expectedSize);
}

static auto ApplyCallRel32PatchExample(const D2RL::PluginContext* context) noexcept -> bool {
	const std::uint32_t expectedSize = ByteCount(ExpectedCallPatch);
	return context->PatchCallRel32(0x0002EB5B, ExpectedCallPatch, expectedSize, 0x00079CD0, expectedSize);
}

static constexpr MemoryPatchExample MemoryPatchExamples[] {
	{ "PatchBytes example failed.",     ApplyBytesPatchExample     },
	{ "PatchFill example failed.",      ApplyFillPatchExample      },
	{ "PatchNop example failed.",       ApplyNopPatchExample       },
	{ "PatchWriteU8 example failed.",   ApplyWriteU8PatchExample   },
	{ "PatchWriteU16 example failed.",  ApplyWriteU16PatchExample  },
	{ "PatchWriteU32 example failed.",  ApplyWriteU32PatchExample  },
	{ "PatchWriteU64 example failed.",  ApplyWriteU64PatchExample  },
	{ "PatchJmpRel32 example failed.",  ApplyJmpRel32PatchExample  },
	{ "PatchCallRel32 example failed.", ApplyCallRel32PatchExample },
};

static auto ApplyPatchExamples(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	// Check every target first. If one expected byte range is wrong, do not partially patch.
	if (!VerifyPatchInputs(context)) {
		context->LogError("One or more memory patch expected byte ranges did not match.");
		return false;
	}

	for (const MemoryPatchExample& example : MemoryPatchExamples) {
		if (!example.apply(context)) {
			context->LogError(example.failureMessage);
			return false;
		}
	}

	return true;
}

static auto InstallHookExample(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	// This example calls the original trampoline and returns the original result.
	const std::uint32_t expectedSize = ByteCount(ExpectedGetVisibleLineCount);
	return context->InstallInlineHook(GetVisibleLineCountRva, ExpectedGetVisibleLineCount, expectedSize, HookGetVisibleLineCount, &OriginalGetVisibleLineCount);
}

D2RL_PLUGIN_EXPORT auto D2RLoaderGetPluginInfo() noexcept -> const D2RL::PluginInfo* {
	return &PatchingSampleInfo;
}

D2RL_PLUGIN_EXPORT auto D2RLoaderLoadPlugin(const D2RL::PluginContext* context) noexcept -> bool {
	if (context == nullptr) {
		return false;
	}

	context->LogInfo("patching sample loaded.");

	if (!context->RegisterConsoleCommand("patching-sample", PatchingSampleCommand, "Print patching sample status.")) {
		context->LogWarn("Patching sample console command was not registered.");
	}

	if (ApplyMemoryPatchExamples) {
		if (!ApplyPatchExamples(context)) {
			return false;
		}

		context->LogInfo("Memory patch examples applied.");
	} else {
		context->LogInfo("Memory patch examples are disabled in source.");
	}

	if (InstallNativeHookExample) {
		if (!InstallHookExample(context)) {
			context->LogError("Native hook example failed.");
			return false;
		}
		context->LogInfo("Native hook example installed.");
	} else {
		context->LogInfo("Native hook example is disabled in source.");
	}

	return true;
}

D2RL_PLUGIN_EXPORT void D2RLoaderUnloadPlugin() noexcept {}
