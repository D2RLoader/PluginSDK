# Namespaced strings

Namespaced strings let you add text by choosing a key. You do not need to find
an unused numeric ID. D2RLoader and plugins always use namespaces. Mods opt in;
existing mods keep the game's native string loading by default.

## Enable namespaces for a mod

`d2rl init` enables namespaces when it creates a mod's metadata. It also prints
examples for native string references. Restart D2RLoader after initialization;
the active localization setting is read at launch. Update copied native
references to use `d2r:`, then rebuild existing BINs before testing.

Already initialized mods keep their metadata when you run `d2rl init` again.
To opt one in, add the top-level `localization` setting to its
`d2rloader/metadata.json`:

```json
{
  "schemaVersion": 1,
  "localization": "namespaced"
}
```

Keep any other metadata fields you already use. The setting belongs beside
`schemaVersion`, outside the descriptive `metadata` object. Both the offline
compiler and in-game compiler preserve it.

Without this setting, the game loads the mod's usual string filenames and
keeps their numeric `id` values, duplicate rules, and native limits. Existing
unqualified layout and table references keep their meaning. These strings use
`d2r:` internally. Updates and automatic setup keep native loading when creating
missing metadata. Having metadata or compiling a mod does not opt it in.
Mods using native loading do not need a `(listfile)` for localization.

The rest of this guide describes namespaced strings. Once opted in, each
package has its own namespace, so two packages can both define `Title`.

| Owner | Full key example |
| --- | --- |
| Original game | `d2r:strCancel` |
| D2RLoader | `d2rloader:D2RLoaderSettingsGeneral` |
| Plugin with ID `sample` | `plugin.sample:Title` |
| Opted-in mod named `My Mod` | `mod.My%20Mod:Title` |

The loader gets the namespace from the package identity, never the filename.
For plugins, this is `PluginInfo.id`. For mods, it is `metadata.name` in
`d2rloader/metadata.json`, or the mod's `modinfo.json` name when metadata has no
name. Spaces and reserved bytes in identities are percent-escaped. Names and
keys are case-sensitive.

## Add text

Place a JSON array below `data/local/lng/strings/` in your mod MPQ or MPQ folder.
Plugins should use their owned resource path, for example
`data/local/lng/strings/d2rloader/sample/text.json`.
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

In a layout supplied by the same package, use `@Title`. In the plugin
localization service, pass `Title` to `getStringByKey`. A local reference only
looks in its own package. To use another package's text, include its namespace:
`@d2r:strCancel` in a layout or `d2r:strCancel` in the service.
Child layouts retain their source package when the loader combines layouts.
Copied native layouts must qualify the native text they retain with `d2r:`.

Packed MPQs must contain a `(listfile)`. This is the archive's file list, which
lets D2RLoader discover arbitrary string filenames. D2RLoader's normal packing
commands include it by default.

## Replace existing text

Put the full target key in a string file in your own mod. You can change loader,
plugin, and original game text from the same file:

```json
[
  { "Key": "d2rloader:D2RLoaderSettingsGeneral", "enUS": "My mod settings" },
  { "Key": "plugin.sample:Title", "enUS": "My plugin settings" },
  { "Key": "d2r:strCancel", "enUS": "Go back" }
]
```

These targets must already exist. The `plugin.sample:Title` example requires
the `sample` plugin to define `Title`. Only the supplied languages change.
Other translations remain intact. You can keep these overrides in any string
file; you do not need to copy the loader's `d2rloader.json` out of its MPQ.

Namespaced mod overrides take priority over plugin overrides. If two plugins override the
same key and language, loading fails with a message naming both sources.
Changing file order does not choose a winner.

## Use text in game tables

For localization columns such as `namestr`, use `SwordName` for your own text
or a qualified key for another package. Do not put `@` in table cells.
When copying a native table, prefix the native string references you retain
with `d2r:`. Other table columns keep their usual meaning.

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
are rejected. Rebuild after changing the localization setting or the mod's
namespace name too. The loader rejects BINs built for a different namespace.

## Reloads and plugin lifetime

Language changes refresh the selected translations. Active resource
registrations refresh plugin strings. Closing a plugin removes its definitions
and overrides. A package that still targets a removed definition must be fixed
or closed too. The SDK copies text into your buffer, so an existing copy remains
yours after a language change or plugin unload.
