# D2RLoader Plugin SDK

This is the public C++ SDK for D2RLoader plugins. It includes the headers, CMake
setup, and small examples needed to build a DLL plugin. Plugins do not need to
use D2RLoader's internal code.

Start with `D2RLPlugin/api.h` and the example closest to what you want to build.
Most plugins do not need to include each service header separately.

The current API is v4. Version 2 and 3 plugins still work. D2RLoader treats v2
plugins as shared because v2 did not define client and server roles.

## Requirements

* Windows x64
* CMake 3.28+
* MSVC or clang-cl
* D2RLoader with plugin API v4 support

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

At minimum, a DLL plugin must:

* embed the D2RLoader plugin manifest resource
* export `D2RLoaderGetPluginInfo`
* export `D2RLoaderLoadPlugin`

Export `D2RLoaderUnloadPlugin` when the plugin has cleanup to do.

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
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DD2RLPLUGIN_BUILD_TESTS=ON
cmake --build build --target D2RLPluginTests
ctest --test-dir build --output-on-failure
```

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

Every API v3 or newer plugin must set exactly one role in `PluginInfo::flags`:

- `PluginFlags::Client` is for UI, input, and other local behavior.
- `PluginFlags::Server` is for gameplay rules and character saves.
- `PluginFlags::Shared` is for behavior needed by both.

The host performs both roles. D2RLoader records server and shared plugins in the
character's plugin history. It does not record client-only plugins. In TCP/IP,
shared plugins must match between the host and client by id, version, API,
scope, and compatibility flags. You may combine a role with `ModScopedOnly` and
`NativeHooks`.

## API Versioning

`D2RL_PLUGIN_API_VERSION` is the binary API version used to build the plugin.

D2RLoader checks it before loading the DLL and skips unsupported versions.

API v3 requires a role. Older v2 plugins did not have roles, so D2RLoader treats
them as `PluginFlags::Shared`. They still load, but must match in TCP/IP and are
recorded in the character's plugin history.

API v4 adds explicit duplicate-Unique item creation, atomic existing-item
operations, semantic item interactions, initial location and new gameplay
events, Hardcore/Softcore character-creation metadata, and asynchronous HTTPS
requests.

## Services

Services are the normal way to use D2RLoader features. Query only the services
you need. Check a service table's size before using newer fields.

Some game and UI work must happen at the right time. `ThreadServiceV1` can queue
a callback, which is a function D2RLoader runs for you. See
[Running Work at the Right Time](#running-work-at-the-right-time).

Query a service through the plugin context:

```cpp
const D2RL::InventoryServiceV1* inventory = nullptr;
if (ctx->QueryService(D2RL::ServiceId::Inventory, D2RL::InventoryServiceV1Version, &inventory) == D2RL::ServiceQueryResult::Success
	&& D2RL::HasInventoryServiceV1Field(inventory, D2RL::InventoryServiceV1RequiredSize)) {
	// Use inventory here. D2RLoader still owns it.
}
```

D2RLoader owns service tables. Do not change or free them. Pass your
`PluginContext` to calls that need an owner. You can still query services while
the plugin unloads so you can remove registrations, but you cannot add new live
state after unloading begins.

### Lifecycle

`LifecycleServiceV1` sends one `DataTablesLoadedEvent` after every completed
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

### Resources and Companion MPQs

Use `ResourceServiceV1` to give D2RCore a file from memory. Pass the full virtual
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

The JSON array needs only the strings the plugin replaces. It does not need a
copy of Blizzard's full file. Each record still uses D2R's normal numeric id,
key, and locale fields. D2RCore keeps the plugin value when the stock file later
contains the same string. An active mod can replace the plugin file by using the
same virtual path in its MPQ or MPQ folder.

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

Use `CustomTableServiceV1` for plugin-owned Excel tables. The plugin does not
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

Use `InventoryServiceV1` to find existing items or own one custom player page.

Existing items use safe handles owned by the plugin. Get the local player from a
UI callback, then inspect the cursor, equipment, belt, inventory, cube, trade,
personal stash, custom page, or shared stash. Cursor and equipment lookups return
handles. Inventory searches give the callback a copied `Items::ItemInfo` for
each match. `ItemServiceV1::getItemInfo` can refresh one handle later. Normal
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

Use `ItemServiceV1` to work with items without a native hook. Queue changes with
`ThreadServiceV1::runOnGameThread`. Only a local game or TCP/IP host may change
game items. A TCP/IP client sends a plugin message to the host. The host uses
`getPeerPlayer` to find that client, then changes items through the returned
player handle.

`createItem` starts with an item code and quality. It can also set the item
level, a SetItems or UniqueItems row, up to three prefix and suffix ids,
quantity, durability, sockets, identified or ethereal state, fixed seeds, and
extra properties. It can place the item automatically or at an exact position
in the inventory, cube, personal stash, custom page, cursor, or ground. An
unsupported destination returns `Unsupported` before the item appears.

Properties use the same format as cube recipes. `propertyId` is the numeric
`*Id` from `Properties.txt`, not an `ItemStatCost.txt` stat id. `parameter` uses
the meaning defined by that property row and is zero for many normal properties.
`minimum` and `maximum` set the roll range. Use
`MakeExactProperty(propertyId, value, parameter)` for one exact value. One
property may update several D2R stats. The `item-transaction` sample uses
Properties `*Id 0` for exactly `+25 Defense`.

`generationSeed` controls item generation. `itemSeed` is stored on the item.
Both must be zero in `Random` mode. `Deterministic` mode supplies both and checks
that D2R kept them. Prefix and suffix ids use D2R's one-based MagicAffix ids. Set
and Unique rows are zero-based. `RandomQualityRecord` lets D2R choose. D2R's
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
cover the normal inventory, Cube, personal stash,
and the current custom page. Equipment, cursor, belt, shared-stash, trade,
corpse, and ground moves are not accepted in V1.

Every operation is validated before mutation. Moved items are removed from a
temporary occupancy model first, so a single transaction can swap or chain
their locations. If a native placement or postcondition fails, D2RLoader
restores moved items and edited values. The handles, runtime ids, seeds, sockets,
and unrelated item data stay intact. `failureIndex` identifies the rejected
operation.

```cpp
D2RL::Items::ExistingItemOperation operations[2] {};
operations[0].structSize     = D2RL::Items::ExistingItemOperationSize;
operations[0].kind           = D2RL::Items::ExistingItemOperationKind::Debit;
operations[0].item           = resourceStack;
operations[0].debit.quantity = 1;

operations[1].structSize                  = D2RL::Items::ExistingItemOperationSize;
operations[1].kind                        = D2RL::Items::ExistingItemOperationKind::Move;
operations[1].item                        = rewardItem;
operations[1].move.destination.structSize = D2RL::Items::ItemDestinationSize;
operations[1].move.destination.container  = D2RL::Items::ItemContainer::Cube;
operations[1].move.destination.placement  = D2RL::Items::Placement::Automatic;

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

`editNativeItem` is for changes V1 cannot describe. It requires
`PluginFlags::NativeHooks` and gives a native pointer to a game callback. The
pointer expires when the callback returns. D2RLoader does not check or publish
raw changes. Normal item inspection, creation, editing, deletion, and exchanges
do not need `NativeHooks`.

### Panels, Layouts, and Widgets

Use `PanelServiceV1` to register a named panel and open or close it. The name is
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

Use `NetworkServiceV1` for small private plugin messages in local and TCP/IP
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
`NetworkServiceV1RequiredSize` before using the full V1 table.

### Input Actions

`InputServiceV1` registers named actions, not raw keyboard hooks. They appear in
D2R's Controls menu and keep saved bindings while the plugin is missing. The
Controls menu clears conflicts with D2R actions and other plugin actions. Plugin
actions do not run while chat, another text field, or the binding control is
active. D2RLoader calls the press and release handlers during input processing.
Queue any UI or game work they need. Returning `Handled` consumes the key press.
Register actions while the plugin loads; D2RLoader removes them when it unloads.

### Shared UI Events

`SharedEventServiceV1` provides common tooltip and panel-message events. A
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

`ItemInteractionServiceV1` reports a logical item activation before D2R handles
it. The event contains generation-safe item and player handles, the actual item
container, the selected cell, keyboard modifiers, and whether the active input
source is keyboard/mouse or controller. V1 emits `Activate` from proven normal
inventory, Cube, personal-stash, and custom-page grids. It deliberately excludes
vendor, trade, corpse, ground, equipment, cursor, belt, and shared-stash paths
until their native behavior is proven.

Callbacks run on the UI thread. Higher priority runs first; equal priorities use
registration order. Return `Continue` to leave the item action alone. Return
`Consume` to stop lower-priority listeners and the normal D2R action. Handles
belong to the receiving plugin, and all registrations are removed automatically
when that plugin unloads. See `item-interactions` for a complete listener.

### Patch Diagnostics

`DiagnosticsServiceV1` compares expected bytes with the running game. A changed
range is `Tracked` when it overlaps a plugin patch or hook known to D2RLoader.
Otherwise it is `Untracked`. The result includes the change type, owner count,
and plugin id when there is one known owner.

### Game Rules

`GameRuleServiceV1` returns the rules used by the current game: maximum sockets,
maximum stack size, skill cap, and whether a player can spend a skill point. The
skill check does not spend the point. Read item and skill limits from a UI or
queued game callback. `canAllocateSkill` needs a game callback.

### HTTPS Requests

`HttpServiceV1` sends GET, POST, PUT, PATCH, DELETE, and HEAD requests to
`https://` URLs. `send` copies the URL, headers, and body before returning, then
runs the request on a worker thread. The response callback also runs on that
worker thread, never the UI or game thread. Queue UI or game work through
`ThreadServiceV1` when a response needs to affect D2R.

Normal certificate and host-name checks stay enabled. Redirects may remain on
HTTPS but cannot downgrade to HTTP. A transport success may still have an HTTP
error status such as 404. The response headers, strings, and body are borrowed
and valid only during the callback. Zero timeout and response-limit fields use
the documented defaults. `cancel` suppresses a callback that has not started;
D2RLoader also cancels every outstanding request when the plugin unloads.

### Running Work at the Right Time

D2R needs some work to happen during a UI update or game update. You do not need
to create or manage threads. Use `ThreadServiceV1::runOnUiThread` or
`ThreadServiceV1::runOnGameThread` to queue a callback, and D2RLoader runs it at
the right time.

Queuing returns right away, and each callback runs once. D2RLoader drops waiting
callbacks when the plugin unloads. It also drops waiting game callbacks when the
game session changes. Change items and use native item pointers only from a game
callback. A remote TCP/IP client must ask the host to make those changes.

### Existing Widgets

`WidgetServiceV1` finds existing panels and child widgets by name. Its handles
store plugin-owned paths, not game pointers, so they still work after a panel is
reopened. From a UI callback, a plugin may read a local rectangle, change
visibility or enabled state, and send a normal target/command/text action.

### Localization

`LocalizationServiceV1` copies active UTF-8 text by numeric id or string key.
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

`ExecuteConsoleCommand` accepts a null-terminated command up to
`MaxConsoleCommandLength`. It briefly enables D2R's cheat flag, sends the
command, waits for dispatch, and restores the old flag before returning. A
`true` result means D2RCore accepted the command, not that the command succeeded.
Normal host, operator, and active-mod rules still apply. Call it from the UI, or
queue it with `ThreadServiceV1::runOnUiThread`.

Keep using `PluginContext` to register commands and write to the console. It
tracks ownership and cleans up when the plugin unloads. If you call a public
export directly, your plugin must resolve it or link its import library. The SDK
header only defines the function.

Ordinal 100 begins D2RCore's private loader API and is not part of the Plugin
SDK. Supported low-level game functions use the separate 2000-2999 range.

`DataTableServiceV1` gives read-only access to the active compiled Excel tables.
It does not expose the loader's main `DataTables` object. Query
`ServiceId::DataTable`, choose `Classic`, `Lod`, or `Rotw`, then request a table
by its stable `TableId`.

Each `TableView` reports the row pointer, count, size, bank, and load revision.
`getRow` reads by physical row index. `findRowById` also supports the logical
ids in `Skills` and `Levels`; `Items` and `ItemTypes` use their row indexes.
`findRowByCode` supports `Items` and `ItemTypes`. Other keyed lookups return
`Unsupported`.

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
