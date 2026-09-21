#pragma once

// SDK release string, also used by CMake for the project and package version.
#define D2RL_SDK_VERSION "0.3.0"

// Oldest plugin ABI the loader supports.
#define D2RL_PLUGIN_MIN_ABI_VERSION 2

// Current plugin ABI: the agreement on data layout and calls between a DLL and the loader.
#define D2RL_PLUGIN_ABI_VERSION     4

// Plugin ABI 3 introduced execution roles. ABI 2 plugins default to Shared.
#define D2RL_PLUGIN_ROLES_ABI_VERSION 3

// The ABI 4 release added the HTTPS service.
#define D2RL_PLUGIN_HTTP_ABI_VERSION 4

#define D2RL_PLUGIN_EXPORT extern "C" __declspec(dllexport)
