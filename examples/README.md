# D2RLoader Plugin SDK Examples

These examples are small and focused. Start with `hello-console`, then choose
the example closest to what you want to build.

## Hello console plugin

`hello-console` is the smallest complete DLL plugin. It exports its plugin info,
registers `hello-plugin`, and writes one console message.

Build it with:

```powershell
cmake --build build --target D2RLHelloConsolePlugin
```

## Data tables plugin

`data-tables` adds a custom TXT file with two columns. After the tables load, it
copies and logs its rows. It also reads the RotW `Levels` table and finds level
124. It does not use D2RLoader's internal `DataTables` object.

Build it with:

```powershell
cmake --build build --target D2RLDataTablesSamplePlugin
```

## Gameplay lifecycle plugin

`gameplay-lifecycle` listens for joining and leaving a game, the local player
becoming ready, initial and later act/area changes, character-level changes,
quest completion, and resurrection. It logs each event. It also shows when
handles from the previous game stop being valid.

Build it with:

```powershell
cmake --build build --target D2RLGameplayLifecycleSamplePlugin
```

## Item transaction plugin

`item-transaction` shows a complete item exchange without native hooks. The
`item-sample-trade` command finds three El runes in the normal inventory and
replaces them with an identified ethereal cap. The cap has two sockets and
exactly `+25 Defense`. If any step fails, the runes are restored.

Build it with:

```powershell
cmake --build build --target D2RLItemTransactionSamplePlugin
```

## Item interactions plugin

`item-interactions` listens before an item is activated in a proven inventory
grid. It logs the safe item/player handles, container, selected cell, input
source, and held modifiers, then returns `Continue` so the stock action still
runs.

Build it with:

```powershell
cmake --build build --target D2RLItemInteractionsSamplePlugin
```

## Game rules plugin

`game-rules` finds the local player and one stored item, then reports the current
socket, stack, skill-cap, and skill-point rules. Run `game-rules-sample` in a
local game or on a TCP/IP host.

Build it with:

```powershell
cmake --build build --target D2RLGameRulesSamplePlugin
```

## Network ping plugin

`network-ping` opens a private message channel for the plugin. The
`network-sample-ping` command connects and sends a ping. The host finds the
player who sent it and replies with a pong.

Build it with:

```powershell
cmake --build build --target D2RLNetworkPingSamplePlugin
```

## HTTPS plugin

`https` registers the `https-sample` command. It queues a GET request to
`https://example.com/` and writes the result, status, header count, and body size
to the plugin log from the response callback.

Build it with:

```powershell
cmake --build build --target D2RLHttpsSamplePlugin
```

## UI panel plugin

`ui-panel` loads a panel layout from memory. The `ui-panel-sample` command opens
and closes it. This shows the Resource and Panel services without a companion
MPQ.

Build it with:

```powershell
cmake --build build --target D2RLUiPanelSamplePlugin
```

## Widget and localization plugin

`widget-localization` opens a panel with D2R's normal frame. Its button uses the
current translation of `strCancel`. The sample finds the button by name, enables
it, reads its position, and prints the translated text to the console and log.
Run `widget-localization-sample` in game.

Build it with:

```powershell
cmake --build build --target D2RLWidgetLocalizationSamplePlugin
```

## Input action plugin

`input-action` adds an action to D2R's Controls menu. Its default key is F8.
Pressing it writes a message to the log and handles the key so D2R does not use
the same press again. The saved key stays with the action after plugin updates.

Build it with:

```powershell
cmake --build build --target D2RLInputActionSamplePlugin
```

## Shared events plugin

`shared-events` adds a white line below Durability on an item tooltip. If the
item has no Durability, the line goes at the bottom of the attributes. It also
adds text to the keybind/action footer and listens for one UI message. Run
`shared-events-sample` to see the message in the console, log, and event
callback.

Build it with:

```powershell
cmake --build build --target D2RLSharedEventsSamplePlugin
```

## Config file plugin

`config-file` embeds its default TOML file with `d2rlplugin_embed_config`.
D2RLoader creates the user's config if it is missing. The example also shows:

* `ReadConfig`
* `WriteConfig`

Build it with:

```powershell
cmake --build build --target D2RLConfigFileSamplePlugin
```

## Runtime patching plugin

`patching` shows runtime patches, expected-byte checks, patch ownership reports,
and the native inline-hook helper. This is an advanced example.

The patch examples are safe as shipped: they write the same bytes already in
the target image. The plugin checks every target with `CheckExpectedBytes`
before writing any of them.

The inline-hook example is off by default. To test it, set
`InstallNativeHookExample` to `true` in
`patching/patching_sample_plugin.cpp`. A plugin that installs inline hooks must
declare `D2RL::PluginFlags::NativeHooks`.

Build it with:

```powershell
cmake --build build --target D2RLPatchingSamplePlugin
```

## DLL-less JSON patches

`json-patches/example-patches.json` shows a simple patch without a DLL. As
shipped, it writes the same bytes that are already in D2R.

To test it manually, place it in one of these folders:

```text
<game>/d2rloader/patches/
<game>/mods/<mod>/d2rloader/patches/
```

D2RLoader only loads strict `*.json` files from those folders.

## Service coverage

Every V1 service has a focused example. Some examples combine services when the
feature needs both.

| Service | Public example |
|---|---|
| Lifecycle | `data-tables`, `gameplay-lifecycle` |
| Resource | `data-tables`, `ui-panel`, `widget-localization` |
| Custom Table | `data-tables` |
| Panel | `ui-panel`, `widget-localization` |
| Inventory | `item-transaction`, `game-rules` |
| Network | `network-ping` |
| HTTPS | `https` |
| Input | `input-action` |
| Data Table | `data-tables` |
| Shared Event | `shared-events` |
| Diagnostics | `patching` |
| Game Rule | `game-rules` |
| Widget | `widget-localization`, `shared-events` |
| Work queue (`ThreadServiceV1`) | `item-transaction`, `network-ping`, `game-rules` |
| Localization | `widget-localization` |
| Item | `item-transaction` |
| Item Interaction | `item-interactions` |

## Plugin manifest

Every v2 or newer DLL plugin needs `D2RL_PLUGIN_MANIFEST_RESOURCE_ID`. It is an
`RCDATA` DWORD containing `D2RL_PLUGIN_API_VERSION`. Each DLL example has a
matching `.rc` file:

```cpp
D2RL_PLUGIN_MANIFEST_RESOURCE_ID RCDATA { D2RL_PLUGIN_RESOURCE_DWORD(D2RL_PLUGIN_API_VERSION) }
```

A missing manifest or old v1 manifest is not compatible.

## Contract tests

The SDK also compiles each public header by itself and checks important ids,
handles, public data layouts, item-code helpers, and property helpers.

```powershell
cmake --build build --target D2RLPluginTests
ctest --test-dir build --output-on-failure
```
