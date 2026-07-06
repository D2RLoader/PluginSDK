# D2RLoader Plugin SDK Examples

This folder has small buildable plugin examples.

## Hello console plugin

`hello-console` is the smallest DLL plugin. It exports plugin info, registers `hello-plugin`, and writes a console message.

Build it with:

```powershell
cmake --build --preset debug --target D2RLHelloConsolePlugin
```

## Config file plugin

`config-file` shows the plugin config helpers:

* `EnsureConfig`
* `ReadConfig`
* `WriteConfig`

Build it with:

```powershell
cmake --build --preset debug --target D2RLConfigFileSamplePlugin
```

## Runtime patching plugin

`patching` shows runtime patch helpers, expected byte checks, and the native inline hook helper.

The patch examples are harmless: they write the same bytes already present in the target image. The plugin checks every target with `CheckExpectedBytes` before applying the group.

Inline hook installation starts disabled. To test it, set `InstallNativeHookExample` to `true` in `patching/patching_sample_plugin.cpp`. Plugins that install inline hooks need `D2RL::PluginFlags::NativeHooks`.

Build it with:

```powershell
cmake --build --preset debug --target D2RLPatchingSamplePlugin
```

## DLL-less JSON patches

`json-patches/example-patches.json` is a DLL-less memory patch example. It only writes bytes already present in the current patched D2R image.

To test it manually, place it in one of these folders:

```text
<game>/d2rloader/patches/
<game>/mods/<mod>/d2rloader/patches/
```

D2RLoader only loads strict `*.json` files from those folders.

## Plugin manifest

Every v2+ DLL plugin needs `D2RL_PLUGIN_MANIFEST_RESOURCE_ID` as an `RCDATA` DWORD containing `D2RL_PLUGIN_API_VERSION`. Each DLL example has a matching `.rc` file:

```cpp
D2RL_PLUGIN_MANIFEST_RESOURCE_ID RCDATA { D2RL_PLUGIN_RESOURCE_DWORD(D2RL_PLUGIN_API_VERSION) }
```

Missing manifests and old v1 manifests are treated as incompatible alpha plugins.
