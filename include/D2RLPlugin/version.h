#pragma once

// The plugin API version is the ABI version. Build plugins against the oldest
// SDK version they support.

#define D2RL_PLUGIN_MIN_API_VERSION 2
#define D2RL_PLUGIN_API_VERSION     4

// API v3 introduced execution roles. Supported API v2 plugins default to Shared.
#define D2RL_PLUGIN_ROLES_API_VERSION 3

// API v4 added item-creation policy flags, richer gameplay/UI events, and the
// asynchronous HTTPS service.
#define D2RL_PLUGIN_HTTP_API_VERSION 4

#define D2RL_PLUGIN_EXPORT extern "C" __declspec(dllexport)
