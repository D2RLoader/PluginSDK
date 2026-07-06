#pragma once

// The plugin API version is the ABI version. Build plugins against the oldest
// SDK version they support.

#define D2RL_PLUGIN_MIN_API_VERSION 2
#define D2RL_PLUGIN_API_VERSION     2
#define D2RL_PLUGIN_EXPORT          extern "C" __declspec(dllexport)
