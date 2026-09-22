# D2RLoader Plugin SDK

This is the public C++ SDK for D2RLoader plugins. It includes the headers, CMake
setup, and small examples needed to build a DLL plugin. Plugins do not need to
use D2RLoader's internal code.

Start with `D2RLPlugin/api.h` and the example closest to what you want to build.
Most plugins do not need to include each service header separately.

**SDK release: 0.3.0 · Plugin ABI: 4**

The SDK release tells you which headers and build tools you have. The plugin ABI
is the agreement on data and function calls between a plugin DLL and the loader.
The current loader also accepts plugin ABI 2 and 3. Check that the services your
plugin needs are available too.
See [SDK and ABI Versioning](#sdk-and-abi-versioning).

## Requirements

* Windows x64
* CMake 3.29+
* MSVC or clang-cl
* D2RLoader with plugin ABI 4 support and the services your plugin requires

## Start a Plugin

Add the SDK to your build, then include:

```cpp
#include <D2RLPlugin/api.h>
```

`api.h` is the normal starting point. If you want narrow includes, use this as
the map:

| Header | Use it for |
|---|---|
| `services.h` | Stable service ids and service-query results. |
| `handles.h` | Safe `PlayerHandle` and `ItemHandle` values used by inventory, item, and network services. |
| `lifecycle_events.h` | Events for completed table loads and gameplay changes. |
| `resources.h` | Files stored in memory under the plugin's resource path. Singular `resource.h` only contains Windows resource ids. |
| `custom_tables.h` | Plugin-owned TXT/BIN table layouts and copied rows. |
| `data_tables.h` | Read-only access to D2R's compiled tables. |
| `inventory.h` | Existing-item discovery plus custom player pages and their policies. |
| `item.h` | Item inspection, creation, editing, deletion, and all-or-nothing exchanges. |
| `item_interactions.h` | Semantic activation events for proven item UI surfaces. |
| `panels.h` | Plugin panels, stock-panel children, custom inventory grids, and controller routes. |
| `widgets.h` | Safe lookup and basic control of existing UI widgets. |
| `input.h` | Named actions and bindings in D2R's Controls menu. |
| `network.h` | Private plugin messages for local and TCP/IP games. |
| `shared_events.h` | Shared tooltip and UI-message listeners. |
| `diagnostics.h` | Detection and ownership reporting for changed executable bytes. |
| `game_rules.h` | Final socket, stack, and skill-allocation rules. |
| `http.h` | Asynchronous HTTPS requests with copied inputs and bounded responses. |
| `threads.h` | Queue one task for the UI or the game. |
| `localization.h` | Copied active UTF-8 text by id or key. |
| `overlay.h` | Display-only lines, rectangles, and text drawn after the game UI. |
| `plugin_communication.h` | Versioned services and named events shared between plugins. |
| `core_exports.h` | Advanced D2RCore functions at ordinals 1-99. |
| `reimplementation_exports.h` | Advanced game functions using raw objects at ordinals 2000-2999. |

The two export headers are low-level contracts. A normal service-based plugin
does not need them.

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

Keep the default settings in one TOML file and embed it in the DLL:

```cmake
d2rlplugin_embed_config(
	TARGET MyPlugin
	FILE "${CMAKE_CURRENT_SOURCE_DIR}/my-plugin.toml"
)
```

D2RLoader creates `<scope>/d2rloader/config/<plugin-id>.toml` before loading the
plugin, but only when the file is missing. It never replaces an existing user or
mod config. The file passed to `d2rlplugin_embed_config` is the only default copy
you need to maintain. You may also ship a loose copy, but the DLL copy is used
when that file is missing. The embedded file must contain something and must be
no larger than 1 MiB.

Shared plugins can declare which config values affect multiplayer gameplay in
the embedded default file:

```toml
[my-plugin]
enabled = true
monster_scale = 1.25
theme = "dark"

[d2rl]
match = ["my-plugin.enabled", "my-plugin.monster_scale"]
```

D2RLoader compares the effective values at those exact paths. TOML formatting,
comments, key order, and unlisted values do not matter. The embedded list is
authoritative; an edited user config cannot change which values are compared.
List only settings that affect shared gameplay. Do not list secrets or
client-only display settings.

Each path must exist in the embedded defaults and use the same casing. A plugin
can list up to 64 unique paths. Plugins that parse the whole config file with a
strict schema must allow the reserved `d2rl` table. Plugins without
`d2rl.match` keep their existing binary-only compatibility check.

At minimum, a DLL plugin must:

* embed the D2RLoader plugin manifest resource
* export `D2RLoaderGetPluginInfo`
* export `D2RLoaderLoadPlugin`

Export `D2RLoaderUnloadPlugin` when the plugin has cleanup to do.

`PluginInfo::version` should contain a Semantic Versioning 2.0.0 value no longer
than 63 characters. Examples include `1.2.0`, `1.5.8-altfix.1`, and
`1.2.0+rotw`. D2RLoader extracts the first valid version for public
compatibility details. If none is found, it uses `invalid` without preventing
the plugin from loading.

## Build the Examples

Build every example from the SDK root:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

Or build only the example you need:

```powershell
cmake --build build --target D2RLHelloConsolePlugin
```

To run the SDK checks too:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --target D2RLPluginTests
ctest --test-dir build --output-on-failure
```

With the MSVC developer environment open and clang-cl on `PATH`, the Debug workflow runs all three steps:

```powershell
cmake --workflow --preset debug
```

Tests are enabled in the Debug preset and disabled in Release. `ctest --preset debug` runs the tests that have already been built. Use `cmake --build --preset debug-tests` to build the checks first.

The build copies sample plugins under the output `d2rloader` folder.

## Load a Plugin

Put a global plugin in:

```text
<game>/d2rloader/plugins/
```

Put a mod-scoped plugin in:

```text
<game>/mods/<mod>/d2rloader/plugins/
```

Mod plugins load first. If a mod plugin and global plugin use the same id, the
mod copy replaces the global copy while that mod is active.

Every plugin using plugin ABI 3 or newer must set exactly one role in `PluginInfo::flags`:

- `PluginFlags::Client` is for UI, input, and other local behavior.
- `PluginFlags::Server` is for gameplay rules and character saves.
- `PluginFlags::Shared` is for behavior needed by both.

The host performs both roles. D2RLoader records server and shared plugins in the
character's plugin history. It does not record client-only plugins. In TCP/IP,
shared plugins must match between the host and client by id, version, plugin ABI,
scope, and compatibility flags. You may combine a role with `ModScopedOnly` and
`NativeHooks`.

## Boss and Herald modifiers

Use `ModifierPolicy` from `encounters.h` to choose the extra modifiers, auras,
and tier damage bonus for Herald replacements or naturally spawned bosses.
The host applies these settings.

Put the policy in your decision's `modifiers` field and set the matching flag:

- `HeraldDecisionOverrideModifiers` replaces the usual random Herald modifiers
  and tier damage bonus for a Herald replacement.
- `BossDecisionApplyHeraldModifiers` applies the policy and Herald tier bonuses
  to a naturally spawned boss and its clones. This also works with zero clones.

### Choosing modifiers and auras

The two mask fields store allowed choices as bits: each bit is an on/off switch.
`allowedModifierMask` uses the game's monster modifier IDs (MonUMod IDs).
`allowedAuraMask` uses positions in `AuraSkillIds`, starting at zero, not skill IDs.
Only bits listed in `SupportedModifierMask` and `SupportedAuraMask` are accepted.

For example, this policy requests one Aura Enchanted modifier with Might:

```cpp
D2RL::Encounters::ModifierPolicy policy{};
policy.allowedModifierMask = 1U << 30; // Modifier ID 30: Aura Enchanted.
policy.minModifiers = 1;
policy.maxModifiers = 1;
policy.allowedAuraMask = 1U << 0; // Position 0 in AuraSkillIds: Might (skill 98).
policy.minAuras = 1;
policy.maxAuras = 1;
```

`1U << n` turns on bit `n`. Combine choices with `|` to allow more than one.
The example leaves `maxAuraLevel` and `damageBoostPercent` at zero.

### Counts and limits

- `minModifiers` and `maxModifiers` accept 0–9 added modifiers. `minAuras` and
  `maxAuras` accept 0–2 auras from Aura Enchanted. Each minimum must be no greater
  than its maximum.
- The game can add fewer than requested. Some modifiers cannot apply to every
  monster, existing modifiers use up space, and the allowed list may be too small.
- Allowing Aura Enchanted does not guarantee it will be chosen. Aura counts apply
  only when it is chosen. Setting `maxAuras` to zero prevents it from being chosen.
- `maxAuraLevel = 0` uses the game's normal aura-level formula, up to level 99.
  Values from 1–99 set the highest level an added aura can reach.
- `damageBoostPercent` accepts 0–10000. For example, `50` adds a 50% damage bonus.
  Zero disables only this bonus. Auras and other modifiers can still raise damage.

Existing modifiers and the boss's own skills, such as Duriel's Holy Freeze, stay
in place. The aura count limits only auras added by this policy.

The loader copies the policy before using it. These settings stay active for
that monster in the current game, even when its area unloads and loads again.
Ordinary Heralds are unchanged. Naturally spawned bosses keep their boss status,
boss loot, and experience.

## SDK and ABI Versioning

These versions describe different parts of a plugin build:

| Version | Source | Used for |
|---|---|---|
| SDK release version, currently `0.3.0` | `D2RL_SDK_VERSION` in `include/D2RLPlugin/version.h` | Identifies the SDK headers and build tools used to build a plugin. |
| Plugin ABI version, currently `4` | `D2RL_PLUGIN_ABI_VERSION` in the same header | Identifies the data layout and function calls shared by a plugin DLL and the loader. |
| Minimum supported plugin ABI, currently `2` | `D2RL_PLUGIN_MIN_ABI_VERSION` in the same header | The oldest plugin ABI the loader accepts. |
| Service ABI version, such as `EncounterService::AbiVersion = 1` | Each service header | Identifies the data layout and function calls for one service, requested through `QueryService`. |
| Plugin release version, such as `1.2.0` | That plugin's `PluginInfo::version` | Plugin release identity, logs, and multiplayer matching. |

`D2RL_SDK_VERSION` is a string that plugin code can use to log which SDK it was
built with. CMake uses the same value for the project and installed package
version. It is not added to DLL metadata or the game's UI automatically.

The plugin manifest and `PluginInfo::abiVersion` declare the plugin ABI.
`PluginContext::abiVersion` carries the plugin ABI accepted by the loader.
D2RLoader rejects unsupported plugin ABI versions before loading a DLL.

Several SDK releases can use the same plugin ABI while offering different
services. Query each service your plugin needs and check its `serviceSize`
before reading newer fields. Check that a function pointer is set before
calling it. If a required service or field is missing, report the requirement
and fail to load cleanly. An optional feature can stay disabled.

For an installed SDK package, `find_package(D2RLPlugin 0.3.0 REQUIRED)` accepts
that release or a later one with the same major number. All `0.x` releases
share major zero. Use `find_package(D2RLPlugin 0.3.0 EXACT REQUIRED)` to require
that specific release. These build-time checks do not check which services
the running loader provides.

Plugin ABI 3 introduced roles. D2RLoader treats ABI 2 plugins as
`PluginFlags::Shared`. They must match in TCP/IP and are recorded in the
character's plugin history.

### SDK 0.3.0 additions

SDK 0.3.0 adds three features without changing the plugin ABI or the existing
service ABI numbers:

- `TableId::TreasureClasses` exposes D2R's compiled runtime form of
  TreasureClassEx.
- `LifecycleService` can report monster deaths with copied monster and direct
  killer identities.
- `WidgetService::getInputText` copies the current UTF-8 text from an input box.

These are appended service fields. Check the full service size before using
them. A plugin built with SDK 0.2.0 keeps using the older part of each service.

### SDK 0.2.0 source-name changes

Update these names when rebuilding a plugin with SDK 0.2.0:

| Previous name | Current name |
|---|---|
| `D2RL_PLUGIN_API_VERSION` | `D2RL_PLUGIN_ABI_VERSION` |
| `D2RL_PLUGIN_MIN_API_VERSION` | `D2RL_PLUGIN_MIN_ABI_VERSION` |
| `D2RL_PLUGIN_ROLES_API_VERSION` | `D2RL_PLUGIN_ROLES_ABI_VERSION` |
| `D2RL_PLUGIN_HTTP_API_VERSION` | `D2RL_PLUGIN_HTTP_ABI_VERSION` |
| `PluginInfo::apiVersion` and `PluginContext::apiVersion` | `PluginInfo::abiVersion` and `PluginContext::abiVersion` |
| `PluginInfoApiVersionSize` | `PluginInfoAbiVersionSize` |
| `NormalizePluginFlagsForApiVersion` | `NormalizePluginFlagsForAbiVersion` |
| Service types such as `InventoryServiceV1` | `InventoryService` |
| Service version constants such as `InventoryServiceV1Version` | `InventoryService::AbiVersion` |
| Size constants such as `InventoryServiceV1RequiredSize` | `InventoryServiceRequiredSize` |
| Field helpers such as `HasInventoryServiceV1Field` | `HasInventoryServiceField` |

All service types, size constants, and field helpers follow the same naming
change. Service queries now take only the typed output pointer:
`ctx->QueryService(&inventory)`. The free function is
`D2RL::QueryService(ctx, &inventory)`.

Update your C++ files and the `.rc` file that holds the plugin's manifest.
The old names have been removed, so these edits are needed before rebuilding.
Existing compiled DLLs still use the same data layout, manifest bytes, and
entry-point names. The plugin ABI stays at **4**.
Existing service ABI numbers stay at **1**.

## Services

Services are the normal way to use D2RLoader features. Query only the services
you need. Check a service table's size before using newer fields.

Some game and UI work must happen at the right time. `ThreadService` can queue
a callback, which is a function D2RLoader runs for you. See
[Running Work at the Right Time](#running-work-at-the-right-time).

Query a service through the plugin context:

```cpp
const D2RL::InventoryService* inventory = nullptr;
if (ctx->QueryService(&inventory) == D2RL::ServiceQueryResult::Success
	&& D2RL::HasInventoryServiceField(inventory, D2RL::InventoryServiceRequiredSize)) {
	// Use inventory here. D2RLoader still owns it.
}
```

The pointer type selects the service. Each service type defines its `Id` and
`AbiVersion`, so the query passes those values automatically. For example,
`InventoryService::AbiVersion` identifies the Inventory interface in these
headers. Each service has its own independent ABI number.

The query returns `UnsupportedVersion` if the loader knows the service but
does not support the requested ABI. A successful query still needs the field
checks shown above. The raw overload accepts an explicit ID, ABI number, and
`const void**` output for code that needs to request a service without a C++ type.

D2RLoader owns service tables. Do not change or free them. Pass your
`PluginContext` to calls that need an owner. You can still query services while
the plugin unloads so you can remove registrations, but you cannot add new live
state after unloading begins.

### Communication Between Plugins

`PluginCommunicationService` lets one plugin offer a small function table to
another plugin. A function table is a struct containing function pointers. This
avoids direct DLL imports and `GetProcAddress`. D2RLoader checks the provider,
version, and table size before returning the table.

A service is identified by its provider plugin ID and a short service name.
Names use lowercase letters, numbers, `.`, `_`, or `-`. They cannot start or end
with a separator. Each name may contain up to 127 bytes.

This provider publishes a price lookup table:

```cpp
struct PriceService {
	using GetPriceFn = int32_t(__cdecl*)(const char* itemCode) noexcept;

	uint32_t   serviceSize;
	uint32_t   serviceVersion;
	GetPriceFn getPrice;
};

static auto __cdecl GetPrice(const char* itemCode) noexcept -> int32_t {
	// A real plugin can look up itemCode in its own data.
	return itemCode != nullptr ? 100 : 0;
}

static const PriceService prices {
	.serviceSize    = sizeof(PriceService),
	.serviceVersion = 1,
	.getPrice       = GetPrice,
};

static bool PublishPrices(const D2RL::PluginContext* context) noexcept {
	const D2RL::PluginCommunicationService* communication = nullptr;
	if (context->QueryService(&communication) != D2RL::ServiceQueryResult::Success
		|| !D2RL::HasPluginCommunicationServiceField(communication, D2RL::PluginCommunicationServiceRequiredSize)) {
		return false;
	}

	const D2RL::PluginCommunication::PublishServiceRequest request {
		.structSize     = D2RL::PluginCommunication::PublishServiceRequestSize,
		.name           = "price-api",
		.serviceVersion = 1,
		.tableSize      = sizeof(prices),
		.table          = &prices,
	};
	return communication->publishService(context, &request)
		== D2RL::PluginCommunication::Result::Success;
}
```

If that provider's plugin ID is `example.prices`, a consumer can acquire it:

```cpp
static D2RL::PluginCommunication::ServiceLease<PriceService> priceLease;

static bool FindPrices(const D2RL::PluginContext* context) noexcept {
	const D2RL::PluginCommunicationService* communication = nullptr;
	if (context->QueryService(&communication) != D2RL::ServiceQueryResult::Success
		|| !D2RL::HasPluginCommunicationServiceField(communication, D2RL::PluginCommunicationServiceRequiredSize)) {
		return false;
	}

	return D2RL::PluginCommunication::Acquire(
		context,
		communication,
		"example.prices",
		"price-api",
		1,
		sizeof(PriceService),
		&priceLease) == D2RL::PluginCommunication::Result::Success;
}
```

Keep the lease while using the table. Its destructor releases the handle. The
loader also releases forgotten handles when the consumer unloads. A newer
service version must keep the older fields and add new fields at the end. Use a
new service name when compatibility cannot be kept. Calls through the table run
on the consumer's calling thread. The provider must document which threads its
functions support.

Held handles also define unload order. Consumers unload before their providers.
The loader rejects an acquisition that would create a circular dependency. For
example, if plugin B holds a service from plugin A, plugin A cannot also acquire
a service from plugin B until B releases its first handle.

The same API supports named events. A plugin may subscribe before the publisher
loads. `publishEvent` copies up to 1 MiB of data, then runs matching callbacks in
subscription order on the publishing thread. The event pointers are valid only
until the callback returns. Keep callbacks short. Use `ThreadService` if the
work must run on the UI or game thread. If a callback fails, D2RLoader stops
that subscriber's callbacks and continues with other plugins. Event versions
follow the same compatibility rule as service versions.

### Display-only Overlays

`OverlayService` draws simple information after the normal game UI. The loader
decides when drawing runs and owns the drawing state. A plugin receives a
canvas handle instead of low-level ImGui, DirectX, or graphics device pointers.

Register a frame callback while the plugin loads. Drawing calls are valid only
inside that callback and on the same thread. Coordinates start at the top-left
corner of the screen. Colors use values from `0.0` to `1.0` for red, green,
blue, and alpha.

```cpp
static const D2RL::OverlayService* overlay = nullptr;

static void __cdecl DrawOverlay(
	const D2RL::PluginContext* context,
	const D2RL::Overlay::Frame* frame,
	void*) noexcept {
	const D2RL::Overlay::LineRequest line {
		.structSize = D2RL::Overlay::LineRequestSize,
		.canvas     = frame->canvas,
		.start      = { 20.0F, 20.0F },
		.end        = { 220.0F, 20.0F },
		.color      = { 1.0F, 0.8F, 0.2F, 1.0F },
		.thickness  = 2.0F,
	};
	(void)overlay->drawLine(context, &line);
}

static bool RegisterOverlay(const D2RL::PluginContext* context) noexcept {
	if (context->QueryService(&overlay) != D2RL::ServiceQueryResult::Success
		|| !D2RL::HasOverlayServiceField(overlay, D2RL::OverlayServiceRequiredSize)) {
		return false;
	}

	const D2RL::Overlay::CallbackRegistration registration {
		.structSize = D2RL::Overlay::CallbackRegistrationSize,
		.phase      = D2RL::Overlay::Phase::AfterGameUi,
		.priority   = 0,
		.callback   = DrawOverlay,
	};
	D2RL::Overlay::CallbackHandle handle = D2RL::Overlay::InvalidCallbackHandle;
	return overlay->registerFrameCallback(context, &registration, &handle)
		== D2RL::Overlay::Result::Success;
}
```

Higher priority callbacks run first. Equal priorities use registration order.
The loader removes callbacks when their plugin unloads and restores clip state
after every callback. This also happens when a callback fails. The V1 service
cannot capture mouse, keyboard, or controller input. It also does not support
images, custom shaders, or plugin-owned graphics resources. The loader handles
graphics device changes itself.

### Lifecycle

`LifecycleService` sends one `DataTablesLoadedEvent` after every completed
table load, including later reloads. Register listeners while the plugin loads.
D2RLoader runs them during a game update, in registration order, after stock
tables, plugin tables, and its own processing are ready. Keep callbacks short.

The revision starts at 1 and changes after each load. The event pointer is valid
only until the listener returns. D2RLoader removes listeners when the plugin
unloads. A listener may unregister itself to stop future calls. If another call
unregisters a listener while it is running, that call waits for it to finish.
The plugin should still check the state of each custom table it owns.

The same service reports game join and leave, local-player readiness, act and
area changes, character-level changes, quest completion, and resurrection. UI
callbacks run in registration order. Immediately after `LocalPlayerReady`, the
initial `ActChanged` and `LevelChanged` use `-1` as the previous value. A quest
event is emitted only when its `PrimaryGoalDone` flag changes from false to true;
existing completions are baselined when the player becomes ready. Its difficulty
and zero-based quest-state row are in the event. Register one listener for each
event you need while the plugin loads. A new game session makes player and item
handles from the old game invalid.

Monster-death listeners are separate because they run on the authoritative
server thread, not the UI thread. The event is sent after D2R handles the normal
drop. It contains copied game, level, monster, and direct-killer identities.
The killer may be a player, monster, missile, or another unit type. A missing
killer uses `UnitType::Invalid` and `InvalidUnitId`. The callback is for
observation only. Keep it short and queue later work when needed.

### Resources and Companion MPQs

Use `ResourceService` to give D2RCore a file from memory. Pass the full virtual
path and the file bytes. D2RCore copies both, so the plugin may release its input
buffers as soon as the call returns.

Resource paths use forward slashes, begin with `data/`, and include the owning
plugin id. For example:

```text
data/global/excel/base/d2rloader/charm-inv/charminv.txt
```

The `d2rloader/<plugin-id>/` part keeps each plugin's files separate. A
successful registration returns a handle. Use it to check the resource state or
unregister it. D2RCore removes any remaining resources when the plugin unloads.

Resources are chosen in this order, with the first matching path winning:

```text
active mod -> mod-scoped plugin -> global plugin -> D2RLoader -> CASC/base game
```

A mod can replace a plugin default by using the same virtual path in its own MPQ
or MPQ folder. It does not need to edit the plugin package. If that copy is
invalid, unreadable, or too large, loading fails. D2RCore does not try the next
copy.

D2RCore looks beside each plugin DLL for `d2rl-<plugin-id>.mpq`. The DLL should
use the matching name `d2rl-<plugin-id>.dll`. During development, the companion
may be a folder. Use a packed MPQ for release. A plugin can see only files under
its own `d2rloader/<plugin-id>/` path, plus its panel layouts under
`data/global/ui/layouts/<plugin-id>/`. A mod-scoped plugin fully replaces a
global plugin with the same id, so its companion must contain all of its default
files. Put smaller mod overrides in the active mod's own MPQ.

A plugin may replace individual game strings through its own file:

```text
data/local/lng/strings/d2rloader/<plugin-id>/strings.json
```

Each plugin owns a string namespace matching its `PluginInfo.id`, for example
`d2rl-sample`. The `d2rl-` prefix is recommended and is not added automatically.
A local `Key` defines
new text in that namespace. To replace existing text, use its full key, such as
`d2r:strCancel` or `d2rloader:D2RLoaderSettingsGeneral`. Supply only the languages
you want to change. Custom `id` fields are ignored and can be omitted.

Any mod can override plugin text from its string JSON files by
using `<plugin-id>:Key`. It does not need to copy the plugin's file.
Mods choose a namespace for their own strings with top-level `"namespace": "sanctuary"` in
`d2rloader/metadata.json`. Explicit references and overrides work without this field.
Without `namespace`, unqualified mod strings keep native loading and IDs. Excel string references
without a namespace always use `d2r:`; custom references need `namespace:Key`.
Mod overrides win over plugin overrides. Conflicting plugin overrides are errors.
Packed MPQs need a listfile to discover arbitrary string filenames. Native mod
filenames already opened by the game also support overrides without a listfile.

Plugin layout text must use explicit references, such as
`@charm-inv:CharmInvPanelTitle` or `@d2r:strClose`. This keeps text working when
a mod copies the layout. An unqualified plugin layout reference reports an error
in the log and notification center, and that layout is not loaded.
See [Namespaced strings](LOCALIZATION.md) for complete examples.

When packaging with `D2RLCompiler build-plugin`, put the plugin id in
`data/d2rloader/compiler/table-schemas.json` inside the companion folder. The id
must match `PluginInfo.id`. A plugin with strings but no custom tables can use:

```json
{
	"format": 1,
	"plugin": "sp-trading",
	"tables": []
}
```

To package a release, leave the game and run this from the main menu:

```text
d2rl plugin pack <plugin-id-or-file-name>
```

The argument may be the plugin id (`charm-inv`) or its DLL name
(`d2rl-charm-inv` or `d2rl-charm-inv.dll`). The active plugin must use a folder
as its companion. D2RLoader compiles its custom TXT tables, packs the files, and
copies the loaded DLL. It writes the finished pair to
`d2rloader/packages/<plugin-id>/` as `d2rl-<plugin-id>.dll` and
`d2rl-<plugin-id>.mpq`. It does not change the development folder. Packages keep
both TXT and BIN files. The TXT is needed to validate the BIN and is also the
file an active mod can replace.

### Custom Tables

Use `CustomTableService` for plugin-owned Excel tables. The plugin does not
need Fog or game pointers. Give the service a short table name, one or both
banks, the row size, and the column list. V1 supports `Ascii`, `Byte`, `Word`,
and `Dword`.

Custom TXT files use a strict tab-separated format. The first row is the header
and must contain every registered column once. Every data row needs the same
number of cells. The file must end with a line feed. Names and text use printable
ASCII. A number is either empty, which means zero, or an unsigned decimal value
that fits the column size. A file with only a header is a valid empty table.
For an empty table, `getTableInfo` reports zero rows and `copyRows` accepts a
null output with zero bytes. Blank rows or invalid cells reject the whole table
before D2R reads it.

D2RCore adds the plugin id to the name. For example, plugin `charm-inv` and
table `charminv` use these paths:

```text
RotW: data/global/excel/d2rloader/charm-inv/charminv.txt
Base: data/global/excel/base/d2rloader/charm-inv/charminv.txt
```

Register tables while the plugin loads. They begin in the `Staged` state.
D2RCore activates them after the plugin loads, then reads or compiles them after
D2R's normal tables. Each bank has its own state, row count, and revision. Call
`getTableInfo`, allocate a local buffer, then call `copyRows` with that revision.
If a reload happens between the two calls, `copyRows` returns `StaleRevision`
without copying part of the table.

D2RCore manages row memory, BIN checks, TXT compilation, caching, reloads, and
cleanup. The plugin receives only a handle and copied rows. A companion MPQ
normally contains the default TXT. An active mod can replace the TXT or BIN at
the same virtual path. `include/D2RLPlugin/custom_tables.h` lists the V1 limits
and required `structSize` fields.

### Inventory and Custom Player Pages

Use `InventoryService` to find existing items or own one custom player page.

Existing items use safe handles owned by the plugin. Get the local player from a
UI callback, then inspect the cursor, equipment, belt, inventory, cube, trade,
personal stash, custom page, or shared stash. Cursor and equipment lookups return
handles. Inventory searches give the callback a copied `Items::ItemInfo` for
each match. `ItemService::getItemInfo` can refresh one handle later. Normal
item calls never expose a native pointer. Each call checks the plugin, game
session, runtime unit id, and item seed again.

The service can also register one custom player page. Its width and height must
be from 1 through 16 because D2S item coordinates use four bits. After D2R uses
the grid, changing its size returns `Busy`. Restart before testing another size.
A protected page either restores its supported saved items or stops the load
before stock code can discard them.

`executeLocalPlayerMove` supports `CursorToPage` and `PageToCursor`. A put uses
`x` and `y` as the item's top-left cell. A take may use any occupied cell. The
loader handles item size, collisions, placement, rollback, and updates. Call it
from a game callback on the local game or host. TCP/IP clients use the panel
binding path, which sends the request to the host.

The page owner may set an item policy. The callback receives copied item type
codes, including the exact type and matching parent types. Use
`MakeItemTypeFourCC` and `ItemPolicyRequestMatchesType` to compare them. Only
`Allow` accepts a local move. Unknown results reject it. `RemoveOnly` is for
saved-item recovery.

Charm activation is separate from item admission. Choose `StockInventoryOnly`,
`PlayerPageOnly`, or `StockInventoryAndPlayerPage`. Configure it while the page
is staged during plugin load. After a committed move or save restore, D2RCore
reruns the normal inventory update so stats, skills, auras, states, and
requirements are refreshed. There is no pre-save veto yet, so use a disposable
character while testing a new page provider.

### Item Creation and Transactions

Use `ItemService` to work with items without a native hook. Queue changes with
`ThreadService::runOnGameThread`. Only a local game or TCP/IP host may change
game items. A TCP/IP client sends a plugin message to the host. The host uses
`getPeerPlayer` to find that client, then changes items through the returned
player handle.

`createItem` starts with an item code and quality. It can also set the item
level, a SetItems or UniqueItems row, up to three prefix and suffix ids,
quantity, durability, sockets, identified or ethereal state, fixed seeds, and
extra properties. It can place the item automatically or at an exact position
in the inventory, cube, personal stash, shared stash, custom page, cursor, or
ground. An unsupported destination returns `Unsupported` before the item
appears. Outputs created by `executeTransaction` use the same destinations.

Shared-stash writes are an optional feature. Check the capability before using
them:

```cpp
const bool canWriteSharedStash = D2RL::HasItemServiceCapability(
	items,
	D2RL::Items::ItemServiceCapability::SharedStashWrite);
```

Set `sharedStashPage` to the normal tab number, starting at zero. For example,
zero means the first normal shared-stash tab. The loader rejects remove-only
tabs and tabs that are not active. Keep both `ItemCreateSpec::structSize` and
`ItemDestination::structSize` set to their current `Size` constants when the
destination is a shared-stash tab.

Properties use the same format as cube recipes. `propertyId` is the numeric
`*Id` from `Properties.txt`, not an `ItemStatCost.txt` stat id. `parameter` uses
the meaning defined by that property row and is zero for many normal properties.
`minimum` and `maximum` set the roll range. Use
`MakeExactProperty(propertyId, value, parameter)` for one exact value. One
property may update several D2R stats. The `item-transaction` sample uses
Properties `*Id 0` for exactly `+25 Defense`.

`generationSeed` controls item generation. `itemSeed` is stored on the item.
Both must be zero in `Random` mode. `Deterministic` mode supplies both and checks
that D2R kept them. Prefix and suffix ids use D2R's one-based combined MagicAffix
ids, in MagicSuffix, MagicPrefix, then AutoMagic order. They are not row numbers
from one source file. For example, when MagicSuffix has 10 rows, MagicPrefix
table-local ID 1 has combined ID 11. Set and Unique rows are zero-based.
`RandomQualityRecord` lets D2R choose. D2R's
once-per-game rule for each Unique row stays enabled by default, including for
random selection. Set `ItemCreateFlag::AllowDuplicateUnique` only when the
plugin intentionally permits the same Unique row more than once. Set items do
not use that rule.

`editItem` changes supported fields on the existing item. Its handle, runtime
id, socket contents, and other data stay the same. V1 can change quantity,
durability, identified state, item level, socket count, and turn ethereal on. It
cannot safely turn ethereal off. Stored and cursor items can be edited. Other
locations are rejected before any change.

`destroyItem` removes a whole item. `executeTransaction` handles exchanges. It
can consume whole items or part of a stack, create any number of outputs, and
apply everything as one operation. If validation, creation, placement, or space
checks fail, it removes new outputs and restores the inputs. Use
`RejectIfNotEmpty` to protect socketed inputs. Choose `DestroyContents` only when
the socket contents should be deleted too. A non-stackable item has a quantity
of one.

`executeExistingItemTransaction` changes items that must keep their handles and
native identity. Pass a tagged array of `Debit`, `Edit`, and `Move` operations.
A debit must leave a positive quantity; use `executeTransaction` when an input
should be consumed completely. Debits and edits accept stored or cursor items.
Atomic edits cover durability, identified state, and item level. Atomic moves
cover the normal inventory, Cube, personal stash, normal shared-stash tabs, and
the current custom page. Equipment, cursor, belt, trade, corpse, and ground
moves are not accepted. A batch may contain up to 4,096 operations.

Every operation is validated before mutation. Moved items are removed from a
temporary occupancy model first, so a single transaction can swap or chain
their locations, including moves between shared-stash tabs. If a native
placement or postcondition fails, D2RLoader restores every source and
destination. The handles, runtime ids, seeds, sockets, and unrelated item data
stay intact. `failureIndex` identifies the rejected operation.

```cpp
if (!D2RL::HasItemServiceCapability(
		items,
		D2RL::Items::ItemServiceCapability::SharedStashWrite)) {
	return;
}

D2RL::Items::ExistingItemOperation operations[2] {};
auto& toShared = operations[0];
toShared.structSize = D2RL::Items::ExistingItemOperationSize;
toShared.kind = D2RL::Items::ExistingItemOperationKind::Move;
toShared.item = inventoryItem;
toShared.move.destination.structSize = D2RL::Items::ItemDestinationSize;
toShared.move.destination.container = D2RL::Items::ItemContainer::SharedStash;
toShared.move.destination.placement = D2RL::Items::Placement::Automatic;
toShared.move.destination.sharedStashPage = 0; // First normal tab.

auto& fromShared = operations[1];
fromShared.structSize = D2RL::Items::ExistingItemOperationSize;
fromShared.kind = D2RL::Items::ExistingItemOperationKind::Move;
fromShared.item = sharedStashItem;
fromShared.move.destination.structSize = D2RL::Items::ItemDestinationSize;
fromShared.move.destination.container = D2RL::Items::ItemContainer::PersonalStash;
fromShared.move.destination.placement = D2RL::Items::Placement::Automatic;

const D2RL::Items::ExistingItemTransaction transaction {
	.structSize     = D2RL::Items::ExistingItemTransactionSize,
	.player         = player,
	.operationCount = 2,
	.operations     = operations,
};
D2RL::Items::ExistingItemTransactionResult result {
	.structSize = D2RL::Items::ExistingItemTransactionResultSize,
};
items->executeExistingItemTransaction(context, &transaction, &result);
```

`splitStack` moves part of one stored stack to the empty cursor as one atomic
operation. The source keeps the same handle and saved identity. The new cursor
stack receives its own saved identity. The requested quantity must be at least
one and must leave at least one item in the source stack. Equipment, belt,
cursor, ground, and detached items are rejected.

This function was appended to the V1 service table. Check its field size before
calling it so the plugin can still load with an older D2RLoader build.

```cpp
if (D2RL::HasItemServiceField(items, D2RL::ItemServiceSplitStackFieldEnd)) {
	const D2RL::Items::SplitStackRequest request {
		.structSize = D2RL::Items::SplitStackRequestSize,
		.player     = player,
		.sourceItem = stack,
		.quantity   = 5,
	};
	D2RL::Items::SplitStackResult result {
		.structSize = D2RL::Items::SplitStackResultSize,
	};
	const auto split = items->splitStack(context, &request, &result);
}
```

`augmentItemAffix` adds one eligible prefix or suffix to an existing Magic or
Rare item. The item keeps its handle, saved identity, and random seeds. The
target and an optional payment item must be in an inventory, Cube, personal
stash, current custom page, or on the cursor. Shared-stash items are not
supported. The payment can be part of a stack or the whole item.

This function is also optional. Check its field and capability before calling
it:

```cpp
const bool canAddAffix = D2RL::HasItemServiceField(
	items,
	D2RL::ItemServiceAffixAugmentFieldEnd)
	&& D2RL::HasItemServiceCapability(
		items,
		D2RL::Items::ItemServiceCapability::AffixAugment);
if (!canAddAffix) {
	return;
}

const D2RL::Items::AffixAugmentRequest request {
	.structSize      = D2RL::Items::AffixAugmentRequestSize,
	.player          = player,
	.item            = targetItem,
	.paymentItem     = currencyItem,
	.paymentQuantity = 1,
	.selection       = D2RL::Items::AffixSelection::RandomEligible,
	.kind            = D2RL::Items::AffixKind::Either,
};
D2RL::Items::AffixAugmentResult result {
	.structSize = D2RL::Items::AffixAugmentResultSize,
};
const auto status = items->augmentItemAffix(context, &request, &result);
if (status == D2RL::Items::Result::Success) {
	// result.appliedKind, result.appliedAffixId, and result.appliedSlot describe it.
}
```

`RandomEligible` requires `affixId` to be zero. It can choose either side, or
you can request only a prefix or suffix. `ExplicitId` requires that exact side
and a nonzero combined affix ID. Combined IDs use the same numbering as
`ItemInfo`: MagicSuffix rows first, then MagicPrefix rows.

Set `maxPrefixes`, `maxSuffixes`, and `maxAffixes` to zero to use D2R's normal
limits. A plugin can set smaller values for its own crafting rule. Magic items
allow one prefix and one suffix. Rare items allow three of each, with six in
total. Rare jewels keep their native total limit of four.

For a free operation, leave `paymentItem` invalid and `paymentQuantity` zero.
On an ordinary failure, the target and payment stay unchanged. Check
`result.failure` for the specific reason. `RollbackFailed` means a native error
made the final state uncertain. Do not retry affix changes in that game session;
D2RLoader blocks them until the next session as a safety measure.

`editNativeItem` is for changes V1 cannot describe. It requires
`PluginFlags::NativeHooks` and gives a native pointer to a game callback. The
pointer expires when the callback returns. D2RLoader does not check or publish
raw changes. Normal item inspection, creation, editing, deletion, and exchanges
do not need `NativeHooks`.

### Panels, Layouts, and Widgets

Use `PanelService` to register a named panel and open or close it. The name is
local to the plugin. It must use 1 through 64 ASCII letters, digits, hyphens, or
underscores, with no slash. D2RCore copies the name and returns a safe handle.
The plugin never receives a raw widget pointer. The owner can inspect, open,
close, toggle, or unregister the panel. D2RCore unregisters it when the plugin
unloads. Run active panel operations from a UI callback. At other times they
return `Busy`.

`PanelRegistration::flags` uses `PanelFlags`. `CloseOnEscape` makes Escape close
the plugin panel like a stock panel. `GameplayLeftSlot` shares the left side of
the screen with character stats and the Horadric Cube. Opening one panel in that
space closes the current one. D2RCore owns the stock panel ids and pointers. The
plugin only chooses the behavior.

`bindPlayerPageGrid` connects an `InventoryGridWidget` child to a custom player
page. Set `PlayerPageGridBinding::structSize`, pass panel and page handles from
the same plugin, and set `childName` to the layout child name. D2RCore copies the
name and connects the grid when the panel opens. No native pointer is exposed.

While the panel is open, Ctrl+click moves an item between the normal inventory
and custom page. D2RCore checks the page's item policy first. It uses D2R's
normal search for free inventory space and can undo the whole move if it fails.
The plugin does not receive an item pointer or quick-move callback.

`registerChildLayout` adds a plugin widget to a supported stock panel without
replacing the stock JSON. Register it while the plugin loads. Set `stockPanel`
to `StockPanel::PlayerInventory`, leave `reserved` at zero, and give it a local
`localId`. `KeyboardMouseOnly` hides it from controller layouts.
`ControllerOnly` hides it from keyboard and mouse layouts. Do not combine them.
D2RCore returns a `ChildLayoutHandle`; pass it to `unregisterChildLayout` to
remove the registration. Panel and child layouts share the same plugin resource
path, so each needs a different `localId`.

For plugin `charm-inv` and local id `InventoryButton`, the child layout uses the
logical name `charm-inv/InventoryButton`. Its HD file is:

```text
data/global/ui/layouts/charm-inv/InventoryButtonhd.json
```

An active mod can replace these files through the normal resource order. With no
input flag, D2RCore adds `PlayerInventory` children to the HD original and
expansion inventories, including controller versions. A controller-only child
may use `data/global/ui/layouts/controller/<plugin-id>/`. Otherwise D2RCore uses
the shared layout. Low-end sprites still work. Legacy non-HD inventory layouts
are not supported yet. The child file must be strict JSON. Its root may be an
`ImageWidget`, `ButtonWidget`, or another native widget with children. A button
can open a plugin panel directly:

```json
"onClickMessage": "PanelManager:TogglePanel:charm-inv/CharmInvPanel"
```

D2RCore applies the panel layout and grid binding while it opens the panel. If
the binding fails, it closes the panel. D2RCore and D2R still own the stock panel
and added widgets; the plugin receives no widget pointers. Unregistering affects
future layout loads. An existing widget stays until D2R reloads the panel or the
game restarts.

D2RCore builds the full logical panel name as `<plugin-id>/<local-id>`. The
matching layout lives at
`data/global/ui/layouts/<plugin-id>/<local-id>hd.json`, and the JSON
root widget's `name` must exactly match the full logical name. For example,
plugin `charm-inv` and local id `CharmInvPanel` use:

```text
Logical name: charm-inv/CharmInvPanel
Layout: data/global/ui/layouts/charm-inv/CharmInvPanelhd.json
```

Custom child widgets follow the same pattern, such as
`charm-inv/InventoryGrid`. Pass that exact name when binding the widget.

### Local and TCP/IP Messages

Use `NetworkService` for small private plugin messages in local and TCP/IP
games. Register a non-zero local channel id while the plugin loads. D2RLoader
combines it with the plugin id and scope, so another plugin may use the same
number. The client and host connect only when their 64-bit
`compatibilityToken` values match.

Call `connectToHost` after entering a supported game. Local games and the TCP/IP
host connect inside the same process. A TCP/IP client makes a private handshake
with the host. An optional callback reports connecting, connected, rejected,
and disconnected states. It also reports when the game ends. If a connection
times out or returns `HostUnavailable`, call `connectToHost` again to retry.
`sendToHost` sends to the host callback. The host replies to its `PeerHandle`
with `sendToClient`.

For a player change, keep the peer handle, queue a game callback, and call
`getPeerPlayer`. The returned `PlayerHandle` belongs to that connected player,
which may not be the host's local player. Clients cannot look up host peers.
Message ids are plugin-defined `uint16_t` values. D2RLoader copies each payload
before the call returns. A payload may contain at most 224 bytes.

The service is not available on Battle.net. It limits traffic from each
plugin/peer pair and drops invalid packets without ending the game. Do not store
callback data pointers or treat channel and peer handles as game pointers. Check
`NetworkServiceRequiredSize` before using the full V1 table.

### Input Actions

`InputService` registers named actions, not raw keyboard hooks. They appear in
D2R's Controls menu and keep saved bindings while the plugin is missing. The
Controls menu clears conflicts with D2R actions and other plugin actions. Plugin
actions do not run while chat, another text field, or the binding control is
active. D2RLoader calls the press and release handlers during input processing.
Queue any UI or game work they need. Returning `Handled` consumes the key press.
Register actions while the plugin loads; D2RLoader removes them when it unloads.

### Shared UI Events

`SharedEventService` provides common tooltip and panel-message events. A
tooltip listener fills a small UTF-8 buffer with the text it wants to add. It
does not edit D2R's full tooltip string. Choose `Description`, `Attributes`, or
`ActionFooter`, then place the text at the top or bottom. Attribute text may also
go above or below one of these D2R fields: Defense, Damage, Chance to Block,
Durability, Quantity, Strength, Dexterity, or Level requirement, Attack Speed,
or Sockets. If the field is missing, omit the text or move it to the top or
bottom of that section.

Write text in the order it should appear. D2RLoader handles D2R's reversed item
text internally. New item text starts in neutral white so it cannot inherit a
color from nearby text. An inline D2R color marker can change it. Lower slots
appear first when several additions use the same position. Higher priority runs
first; equal priorities use registration order.

UI-message listeners receive copied target, command, and text values. They may
consume D2R messages or Widget-service actions before normal panel handling.
`CharacterCreate:Create` also reports whether the selected character is
Hardcore or Softcore; other UI messages report `Unknown`.
D2RLoader runs both listener types during UI updates. Listeners may unregister
themselves. D2RLoader removes them when the plugin unloads.

### Item Interactions

`ItemInteractionService` reports a logical item activation before D2R handles
it. The event contains generation-safe item and player handles, the actual item
container, the selected cell, keyboard modifiers, and whether the active input
source is keyboard/mouse or controller. V1 emits `Activate` from proven normal
inventory, Cube, personal-stash, custom-page, shared-stash, and belt controls.
Shared-stash and belt events are opt-in. Set the listener's `containerMask` to
`DefaultContainerMask` plus the extra container bits you need. A zero mask keeps
the default inventory, Cube, personal-stash, and custom-page behavior. First
check that the service table contains `supportedContainerMask`. Then request only
the extra bits that field reports. Older loaders do not contain this field and
support only the default containers. Vendor, trade, corpse, ground, equipment,
and cursor paths remain excluded.

Callbacks run on the UI thread. Higher priority runs first; equal priorities use
registration order. Return `Continue` to leave the item action alone. Return
`Consume` to stop lower-priority listeners and the normal D2R action. Handles
belong to the receiving plugin, and all registrations are removed automatically
when that plugin unloads. See `item-interactions` for a complete listener.

### Patch Diagnostics

`DiagnosticsService` compares expected bytes with the running game. A changed
range is `Tracked` when it overlaps a plugin patch or hook known to D2RLoader.
Otherwise it is `Untracked`. The result includes the change type, owner count,
and plugin id when there is one known owner.

Use `enumerateModificationRanges` when you need the exact changed parts. The
first call asks for the number of entries. The next call fills the buffer. If
the number grows between calls, `BufferTooSmall` asks the plugin to resize the
buffer and try again.

```cpp
if (!D2RL::HasDiagnosticsServiceField(
		diagnostics, D2RL::DiagnosticsServiceEnumerateModificationRangesFieldEnd)) {
	return false;
}

const uint8_t expected[] { 0x48, 0x89, 0x5C, 0x24, 0x08 };
const D2RL::Diagnostics::HookQuery query {
	.structSize   = D2RL::Diagnostics::HookQuerySize,
	.rva          = 0x00123456,
	.expected     = expected,
	.expectedSize = sizeof(expected),
};

std::vector<D2RL::Diagnostics::ModificationRange> ranges;
uint32_t rangeCount = 0;
auto result = diagnostics->enumerateModificationRanges(
	context, &query, nullptr, 0, &rangeCount);
while (result == D2RL::Diagnostics::Result::BufferTooSmall) {
	ranges.resize(rangeCount);
	result = diagnostics->enumerateModificationRanges(
		context, &query, ranges.data(), static_cast<uint32_t>(ranges.size()), &rangeCount);
}
if (result != D2RL::Diagnostics::Result::Success) {
	return false;
}
ranges.resize(rangeCount);
```

Each tracked entry names one known patch or hook and its owner. An untracked
entry covers changed bytes with no known owner. `callThrough` is `Yes` only for
a loader-managed inline hook with a working pointer to the original function.
It is `No` when that hook has no working pointer. Byte patches and unknown
changes report `Unknown`.

### Atomic Patch Transactions

`MutationService` groups byte patches, relative calls or jumps, and inline hooks
into one transaction. Staging only copies the request. It does not change game
memory. `commit` first checks every address, expected byte range, overlap, hook
target, and call-through pointer. It then applies the complete group. If a
later step fails, D2RLoader restores the earlier changes.

```cpp
const D2RL::MutationService* mutations = nullptr;
if (context->QueryService(&mutations) != D2RL::ServiceQueryResult::Success
	|| !D2RL::HasMutationServiceField(mutations, D2RL::MutationServiceRequiredSize)) {
	return false;
}

D2RL::Mutations::TransactionHandle transaction = 0;
if (mutations->beginTransaction(context, &transaction) != D2RL::Mutations::Result::Success) {
	return false;
}

const uint8_t expected[] { 0x74, 0x05 };
const uint8_t replacement[] { 0x90, 0x90 };
const D2RL::Mutations::BytePatchRequest patch {
	.structSize   = D2RL::Mutations::BytePatchRequestSize,
	.rva          = 0x00123456,
	.expected     = expected,
	.expectedSize = sizeof(expected),
	.bytes        = replacement,
	.size         = sizeof(replacement),
};
D2RL::Mutations::OperationHandle operation = 0;
if (mutations->stageBytePatch(context, transaction, &patch, &operation) != D2RL::Mutations::Result::Success) {
	mutations->cancelTransaction(context, transaction);
	return false;
}

D2RL::Mutations::CommitResult result {
	.structSize = D2RL::Mutations::CommitResultSize,
};
if (mutations->commit(context, transaction, &result) != D2RL::Mutations::Result::Success) {
	mutations->cancelTransaction(context, transaction);
	return false;
}
```

The service copies `expected` and `replacement` during the staging call, so the
arrays may be local variables. Each byte patch must have matching expected and
replacement sizes. A relative call or jump needs at least 5 bytes. A transaction
cannot overlap another staged operation or an existing loader-tracked patch.

Inline hooks require `PluginFlags::NativeHooks`. Keep the operation handle from
`stageInlineHook`, then call `getInlineHookOriginal` after commit succeeds. This
is the call-through trampoline, which lets the hook call the original function.
D2RLoader does not publish it during a partly completed commit.

`CommitResult::operation` identifies the operation that failed. The
`OriginalStateRestored` flag confirms that no part of the transaction remains
active. `cancelTransaction` discards a transaction that did not commit. A
successful transaction stays active until the plugin unloads, when D2RLoader
restores its patches and removes its hooks.

If commit returns `RollbackFailed`, call `cancelTransaction` once more. It makes
another restore attempt. D2RLoader also retries cleanup when the plugin unloads.

Commit uses one loader lock so loader-managed patches cannot race with one
another. It does not pause every game thread for the whole operation. Game code
running at the same moment may briefly see the changes being applied.

### Game Rules

`GameRuleService` returns the rules used by the current game: maximum sockets,
maximum stack size, skill cap, and whether a player can spend a skill point. The
skill check does not spend the point. Read item and skill limits from a UI or
queued game callback. `canAllocateSkill` needs a game callback.

### HTTPS Requests

`HttpService` sends GET, POST, PUT, PATCH, DELETE, and HEAD requests to
`https://` URLs. `send` copies the URL, headers, and body before returning, then
runs the request on a worker thread. The response callback also runs on that
worker thread, never the UI or game thread. Queue UI or game work through
`ThreadService` when a response needs to affect D2R.

Normal certificate and host-name checks stay enabled. Redirects may remain on
HTTPS but cannot downgrade to HTTP. A transport success may still have an HTTP
error status such as 404. The response headers, strings, and body are borrowed
and valid only during the callback. Zero timeout and response-limit fields use
the documented defaults. `cancel` suppresses a callback that has not started;
D2RLoader also cancels every outstanding request when the plugin unloads.

### Running Work at the Right Time

D2R needs some work to happen during a UI update or game update. You do not need
to create or manage threads. Use `ThreadService::runOnUiThread` or
`ThreadService::runOnGameThread` to queue a callback, and D2RLoader runs it at
the right time.

Queuing returns right away, and each callback runs once. D2RLoader drops waiting
callbacks when the plugin unloads. It also drops waiting game callbacks when the
game session changes. Change items and use native item pointers only from a game
callback. A remote TCP/IP client must ask the host to make those changes.

### Existing Widgets

`WidgetService` finds existing panels and child widgets by name. Its handles
store plugin-owned paths, not game pointers, so they still work after a panel is
reopened. From a UI callback, a plugin may read a local rectangle, change
visibility or enabled state, and send a normal target/command/text action.
`getInputText` accepts an `InputTextBoxWidget` or a derived widget. Call it first
with a null or small output buffer to get `BufferTooSmall` and the required UTF-8
byte count. The count includes the trailing null byte. Call it again with a
large enough buffer. Like the other widget calls, it must run on the UI thread.

### Localization

`LocalizationService` copies active UTF-8 text by key. A local key such as
`Title` belongs to the calling plugin. Use `d2r:strCancel` to read native game
text, or `d2rloader:D2RLoaderSettingsGeneral` to read loader text. Numeric lookup
accepts original game IDs only. The service version is 2.
Call once with a null or small output buffer to get `BufferTooSmall` and the
required byte count, then call again with that size. The count includes the
trailing null byte.

## Advanced: Low-Level D2RCore Access

You can skip this section when starting a plugin. Most plugins should use
`PluginContext` and the services above. Direct D2RCore access is for plugins that
need live D2R objects or another feature the services do not provide.

D2RCore uses ordinals 1-99 for its named public API. There is no ordinal 0.
`D2RLPlugin/core_exports.h` defines the names, ordinals, binary layouts, and
function types. `IsInGame` is ordinal 13. `ExecuteConsoleCommand` is ordinal 14.
`PrepareHostEnvironment` is ordinal 15.

`ExecuteConsoleCommand` accepts a null-terminated command up to
`MaxConsoleCommandLength`. It briefly enables D2R's cheat flag, sends the
command, waits for dispatch, and restores the old flag before returning. A
`true` result means D2RCore accepted the command, not that the command succeeded.
Normal host, operator, and active-mod rules still apply. Call it from the UI, or
queue it with `ThreadService::runOnUiThread`.

`PrepareHostEnvironment` is for dedicated-server plugins that create a game
without D2RLoader's normal hosting screen. After loading every game resource and
Excel table, call it once and then start accepting players. D2RLoader captures
the full local fingerprint, plugin inventory, all gameplay banks, item-stat
schemas, and handshake data as one immutable server-wide manifest. Repeated
calls keep the original frozen manifest.

The dedicated server may host simultaneous Classic, Lord of Destruction, and
Reign of the Warlock games. D2RLoader performs a version-neutral transport
preflight first, then reads the target game's version from D2R when admitting
the player and compares only that game's gameplay bank and item-stat schema.

The error buffer is optional. When supplied, it receives an empty string on
success or a null-terminated explanation on failure, truncated to fit. A
dedicated server must not accept players when preparation returns `false`.

Keep using `PluginContext` to register commands and write to the console. It
tracks ownership and cleans up when the plugin unloads. If you call a public
export directly, your plugin must resolve it or link its import library. The SDK
header only defines the function.

Ordinal 100 begins D2RCore's private loader API and is not part of the Plugin
SDK. Supported low-level game functions use the separate 2000-2999 range.

`DataTableService` gives read-only access to the active compiled Excel tables.
It does not expose the loader's main `DataTables` object. Query
`ServiceId::DataTable`, choose `Classic`, `Lod`, or `Rotw`, then request a table
by its stable `TableId`.

Each `TableView` reports the row pointer, count, size, bank, and load revision.
`getRow` reads by physical row index. `findRowById` also supports the logical
ids in `Skills` and `Levels`; `Items`, `ItemTypes`, and `TreasureClasses` use
their row indexes. `findRowByCode` supports `Items` and `ItemTypes`. Other keyed
lookups return `Unsupported`.

`TreasureClasses` is D2R's compiled runtime form of TreasureClassEx. Its rows
are not `TreasureClassExTxt` rows. The runtime layout contains a dynamic item
list, so always check the reported row size against the layout for the running
game build before reading it.

Rows use the compiled layout for the running game build. Set `structSize` on
every output and check `rowSize` before casting a pointer. Use this service only
from a game callback. Row pointers are read-only and expire when the next table
load starts. Do not store, change, or free them. The service is unavailable
during a load. `DataTablesLoadedEvent` tells the plugin when the new rows are
ready.

`D2RLPlugin/reimplementation_exports.h` lists the public low-level game
functions at ordinals 2000-2999, along with their stability and function types.
The first supported functions are:

* `GetLevelIdFromUnit` at ordinal 2000
* `GetLevelIdFromRoomContext` at ordinal 2001
* `GetItemHandednessForUnit` at ordinal 2002
* `CanEquipItemPair` at ordinal 2003
* `GenerateUnitCastId` at ordinal 2004
* `SetUnitCastId` at ordinal 2005
* `TryGetMonumodConstant` at ordinal 2006
* `CastAmplifyDamage` at ordinal 2007

The level helpers return zero for a null pointer or an object with no current
level. `GetItemHandednessForUnit` returns a named handedness value. It treats a
null or non-item pointer as one-handed. `CanEquipItemPair` accepts a missing item
because one item cannot cause a pair conflict. Its optional flag enables the
standard 3.2 CharStats off-hand rule.

`GenerateUnitCastId` returns `UINT32_MAX` for a null game. Otherwise it advances
the game's cast-group counter and wraps `0x7FFFFFFE` to `1`. `SetUnitCastId`
does nothing for a null unit. Custom missile and skill handlers can use these
helpers for D2R's normal next-hit-delay grouping.

`TryGetMonumodConstant` copies one signed `monumod.constants` value for a game
version. It returns false if the version, table, row, or output is invalid.
`CastAmplifyDamage` applies the retail 3.2 Cursed modifier. Rows 34 and 35 choose
the normal and alternate chance. Monster flags decide whether it can run, then
D2R's normal radius function applies the curse. Its game and unit arguments must
be matching live objects. Passing null does nothing.

Every other non-null argument must point to the right live D2R object. Call these
functions only from a game callback where that object is valid. They cannot make
an old or guessed address safe.

The `data-tables` sample shows the safer service-based path. It registers a
custom TXT file and its compiled layout, logs the custom rows after each load,
reads the RotW `Levels` table, and finds level 124.

## License

The SDK is MIT licensed. You can use it for open-source or closed-source
plugins. See `LICENSE`.

This SDK is not affiliated with, endorsed by, or sponsored by Blizzard Entertainment. Diablo and Diablo II: Resurrected are trademarks or registered trademarks of Blizzard Entertainment.
