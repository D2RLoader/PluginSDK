# Namespaced strings

Namespaced strings let you add text by choosing a key. You do not need to find
an unused numeric ID. D2RLoader and plugins always use namespaces. Any mod can
reference or override their strings. Mods choose whether their own strings use
a separate namespace; ordinary mod strings use native loading by default.

## Enable namespaces for a mod

`d2rl init` enables namespaces when it creates a mod's metadata. It also prints
examples for native string references. Restart D2RLoader after initialization;
the active namespace is read at launch. Copied native layouts need
`d2r:` references. Excel references without a namespace already use `d2r:`.
Rebuild existing BINs before testing.

Already initialized mods keep their metadata when you run `d2rl init` again.
To opt one in, add a top-level `namespace` to its
`d2rloader/metadata.json`:

```json
{
  "schemaVersion": 1,
  "namespace": "sanctuary"
}
```

Keep any other metadata fields you already use. The namespace belongs beside
`schemaVersion`, outside the descriptive `metadata` object. Both compilers
preserve it. `metadata.name` remains your display name and can contain spaces.
`d2rl init` suggests a lowercase namespace from the mod folder name, replacing
spaces with hyphens. You can edit that namespace before adding references.

Without `namespace`, the game loads unqualified records from the mod's usual string
filenames and keeps their numeric `id` values, duplicate rules, and native limits. Existing
unqualified layout and table references keep their meaning. These strings use
`d2r:` internally. Updates and automatic setup keep native loading when creating
missing metadata. Having metadata or compiling a mod does not opt it in.
Explicit `namespace:Key` records are handled as overrides and do not need an `id`.
Native filenames already opened by the game do not need a `(listfile)`.
Additional filenames in packed MPQs need a listfile so they can be discovered.

The rest of this guide describes namespaced strings. Once opted in, each
package has its own namespace, so two packages can both define `Title`.

| Owner | Full key example |
| --- | --- |
| Original game | `d2r:strCancel` |
| D2RLoader | `d2rloader:D2RLoaderSettingsGeneral` |
| Plugin with ID `d2rl-sample` | `d2rl-sample:Title` |
| Mod with namespace `sanctuary` | `sanctuary:Title` |

The filename does not select the namespace. Plugins use `PluginInfo.id` exactly;
the recommended `d2rl-` prefix is optional and is never added automatically.
Mods choose their top-level `namespace` in `d2rloader/metadata.json`.
Use 1-256 ASCII letters, digits, hyphens, underscores, or periods.
An empty or invalid namespace reports an error. Omit the field for native loading. Namespaces
and keys are case-sensitive. `d2r` and `d2rloader` are reserved. A mod and an
active plugin cannot share a namespace; choose a distinct name for each.

## Add text

Place a JSON array below `data/local/lng/strings/` in your mod MPQ or MPQ folder.
Plugins should use their owned resource path, for example
`data/local/lng/strings/d2rloader/d2rl-sample/text.json`.
The filename can be anything ending in `.json`.

```json
[
  { "Key": "Title", "enUS": "My settings", "deDE": "Meine Einstellungen" },
  { "Key": "SwordName", "enUS": "Moonblade" }
]
```

In namespaced string files, `id` fields are ignored, including in overrides. Leave them out of new
files. Define a key only once in your package. Each record needs at least one
translation. Supply every language your package supports: a missing translation
does not fall back to English. An empty translation is allowed and displays no
text.

Plugin layouts must use full references, such as `@d2rl-sample:Title`.
An unqualified reference reports an error in the log and notification center,
and the layout is not loaded. Explicit references keep their meaning when a mod
copies the file. A mod's own layouts can use `@Title` for their local strings.
In the plugin localization service, `getStringByKey` still accepts `Title` because
the caller supplies the plugin identity. To use game text, pass `@d2r:strCancel`
in a layout or `d2r:strCancel` in the service.
Child layouts retain their source package when the loader combines layouts.
Copied native layouts must qualify the native text they retain with `d2r:`.

You can mix full references with other text in a plugin layout:

```json
{ "text": "@d2rl-sample:Title: @d2r:strCancel\nMore text" }
```

Each reference is translated separately. Spaces, punctuation, and line breaks
stay in place. Inline keys use ASCII letters, digits, and underscores. A key
with spaces or other punctuation can still be used as the entire text value.
Translations are inserted once; an `@` inside a translation stays literal.
A colon immediately followed by a key marks a namespace. Put a space after a
literal colon, as in `@d2rl-sample:Title: More text`.

Packed MPQs need a `(listfile)` to discover arbitrary string filenames.
D2RLoader's normal packing commands include this file list by default.

## Replace existing text

Put the full target key in a string file in your own mod. No namespace opt-in is
needed for overrides. You can change loader, plugin, and original game text from
the same file:

```json
[
  { "Key": "d2rloader:D2RLoaderSettingsGeneral", "enUS": "My mod settings" },
  { "Key": "d2rl-sample:Title", "enUS": "My plugin settings" },
  { "Key": "d2r:strCancel", "enUS": "Go back" }
]
```

These targets must already exist. The `d2rl-sample:Title` example requires
the `d2rl-sample` plugin to define `Title`. Only the supplied languages change.
Other translations remain intact. You can keep these overrides in any string
file; you do not need to copy the loader's `d2rloader.json` out of its MPQ.

Mod overrides take priority over plugin overrides. If two plugins override the
same key and language, loading fails with a message naming both sources.
Changing file order does not choose a winner.

## Use text in game tables

For localization columns such as `namestr`, unqualified keys always use the
original game's `d2r:` namespace. This applies to native, mod, loader, and
plugin tables. A local definition with the same key does not change this rule.

| Cell value | String selected |
| --- | --- |
| `cap` | `d2r:cap` |
| `d2r:cap` | `d2r:cap` |
| `sanctuary:SwordName` | Sanctuary's `SwordName` |
| `d2rl-sample:Title` | The plugin's `Title` |

Do not put `@` in table cells. Copied native references can stay unchanged.
Only custom text needs a namespace. Other columns keep their usual meaning.
Some text fields, including affix names, store the key in a fixed-size field;
the full `namespace:key` must fit that field.

The compiler stores qualified keys in D2RLoader BIN files. The loader assigns
the game's two-byte references automatically when those tables load. These
temporary numbers are internal; do not save them or use them in JSON.

Key-based strings can exceed 65,535. Game table fields still have a finite pool
of two-byte references shared by all packages. Original game IDs reserve their
slots, and only custom strings used by tables consume additional slots. A full
pool produces an error instead of wrapping or replacing another string.

The offline compiler checks your local definitions. It preserves references to
other packages for validation when D2RLoader has the complete set of packages.
Rebuild compiled string links with the current compiler; old BIN link formats
are rejected. Rebuild after adding, removing, or changing the mod's namespace. The loader rejects BINs built for a different namespace.

## Reloads and plugin lifetime

Language changes refresh the selected translations. Active resource
registrations refresh plugin strings. Closing a plugin removes its definitions
and overrides. A package that still targets a removed definition must be fixed
or closed too. The SDK copies text into your buffer, so an existing copy remains
yours after a language change or plugin unload.
