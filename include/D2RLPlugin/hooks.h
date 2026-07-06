#pragma once

#include <cstddef>
#include <cstdint>

namespace D2RL {

struct PluginContext;

struct InlineHookRegistration {
	uint32_t    registrationSize;
	uint32_t    flags;
	uint64_t    rva;
	const void* expected;
	uint32_t    expectedSize;
	void*       target;
	void**      original;
	uintptr_t   reserved[4];
};

inline constexpr uint32_t InlineHookRegistrationSize         = static_cast<uint32_t>(sizeof(InlineHookRegistration));
inline constexpr uint32_t InlineHookRegistrationRequiredSize = static_cast<uint32_t>(offsetof(InlineHookRegistration, target) + sizeof(void*));
inline constexpr uint32_t InlineHookRegistrationOriginalSize = static_cast<uint32_t>(offsetof(InlineHookRegistration, original) + sizeof(void**));

inline auto HasInlineHookRegistrationField(const InlineHookRegistration* hook, uint32_t fieldEndOffset) noexcept -> bool {
	return hook != nullptr && hook->registrationSize >= fieldEndOffset;
}

inline auto InlineHookOriginal(const InlineHookRegistration* hook) noexcept -> void** {
	return HasInlineHookRegistrationField(hook, InlineHookRegistrationOriginalSize) ? hook->original : nullptr;
}

inline auto MakeInlineHook(uint64_t rva, const void* expected, uint32_t expectedSize, void* target, void** original = nullptr) noexcept -> InlineHookRegistration {
	return {
		.registrationSize = InlineHookRegistrationSize,
		.rva              = rva,
		.expected         = expected,
		.expectedSize     = expectedSize,
		.target           = target,
		.original         = original,
	};
}

using InstallInlineHookFn = bool(__cdecl*)(const PluginContext* ctx, const InlineHookRegistration* hook) noexcept;

}
