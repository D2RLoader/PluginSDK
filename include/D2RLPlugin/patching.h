#pragma once

#include <cstdint>

namespace D2RL {

struct PluginContext;

enum class Rel32PatchKind : uint32_t {
	Jump = 1,
	Call = 2,
};

using PatchBytesFn         = bool(__cdecl*)(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, const void* bytes, uint32_t size) noexcept;
using PatchFillFn          = bool(__cdecl*)(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint8_t value, uint32_t size) noexcept;
using PatchWriteFn         = bool(__cdecl*)(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t value, uint32_t size) noexcept;
using PatchRel32Fn         = bool(__cdecl*)(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize, uint64_t targetRva, uint32_t size, Rel32PatchKind kind) noexcept;
using CheckExpectedBytesFn = bool(__cdecl*)(const PluginContext* ctx, uint64_t rva, const void* expected, uint32_t expectedSize) noexcept;

}
