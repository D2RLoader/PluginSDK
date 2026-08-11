#pragma once

#include <cstdint>

namespace D2RL {

// These handles never contain native game pointers. They belong to the plugin
// that received them, and a new game session makes them stale.
using PlayerHandle = uint64_t;
using ItemHandle   = uint64_t;

inline constexpr PlayerHandle InvalidPlayerHandle = 0;
inline constexpr ItemHandle   InvalidItemHandle   = 0;

static_assert(sizeof(PlayerHandle) == sizeof(uint64_t));
static_assert(sizeof(ItemHandle) == sizeof(uint64_t));

}
