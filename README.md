# D2RLoader Plugin SDK

C++ headers and examples for D2RLoader plugins.

This repo holds:

* `include/D2RLPlugin` has the plugin ABI and small helper wrappers.
* `examples` has buildable plugin examples.
* `cmake` has the installable CMake package config.

The current plugin API version is v2. Build plugins against the oldest SDK version they support.

## Requirements

* Windows x64
* CMake 3.28+
* MSVC or clang-cl
* D2RLoader with plugin API v2 support

## Using the SDK

Add the SDK to your build or install it with CMake, then include:

```cpp
#include <D2RLPlugin/api.h>
```

Link your plugin to the SDK target:

```cmake
add_subdirectory(path/to/D2RLoader-PluginSDK)

add_library(MyPlugin SHARED my_plugin.cpp my_plugin.rc)
target_link_libraries(MyPlugin PRIVATE D2RLPlugin::D2RLPlugin)
```

If the SDK is installed, use the package instead:

```cmake
find_package(D2RLPlugin CONFIG REQUIRED)

add_library(MyPlugin SHARED my_plugin.cpp my_plugin.rc)
target_link_libraries(MyPlugin PRIVATE D2RLPlugin::D2RLPlugin)
```

A DLL plugin needs to:

* embed the D2RLoader plugin manifest resource
* export `D2RLoaderGetPluginInfo`
* export `D2RLoaderLoadPlugin`

`D2RLoaderUnloadPlugin` may also be exported.

## Building Examples

From the D2RLoader repo:

```powershell
cmake --build --preset debug --target D2RLHelloConsolePlugin
cmake --build --preset debug --target D2RLConfigFileSamplePlugin
cmake --build --preset debug --target D2RLPatchingSamplePlugin
```

From this SDK repo:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Or build one sample:

```powershell
cmake --build build --target D2RLHelloConsolePlugin
```

The build copies sample plugins under the output `d2rloader` folder.

## Loading Plugins

Global plugins go in:

```text
<game>/d2rloader/plugins/
```

Mod-scoped plugins go in:

```text
<game>/mods/<mod>/d2rloader/plugins/
```

Mod plugins load before global plugins. If a mod plugin and a global plugin use the same id, the global plugin is skipped while that mod is active.

## Versioning

D2RL_PLUGIN_API_VERSION is the plugin ABI version.

D2RLoader checks this value before loading a plugin. If the plugin was built for an unsupported API version, it will be skipped.

For best compatibility, build your plugin against the oldest SDK version that has the features you need.

When D2RLoader updates the plugin API, older plugins may need to be rebuilt or updated before they can load again.

## License

MIT licensed. See `LICENSE`.

You can use the SDK for open-source or closed-source plugins.

This SDK is not affiliated with, endorsed by, or sponsored by Blizzard Entertainment. Diablo and Diablo II: Resurrected are trademarks or registered trademarks of Blizzard Entertainment.
