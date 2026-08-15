# Plugin SDK Service Extensions

**Status:** Draft for maintainer discussion

**Author:** RuffnecKk

**Compatibility:** Version-neutral

## Purpose

API v3 resolves most of the service gaps we previously identified. This
proposal focuses on three remaining boundaries observed while migrating a
production multi-DLL pack that governs 192 native hook and patch sites.

This is a functional proposal only. It assigns no service identifier, freezes
no C++ layout, and does not decide whether an accepted change should extend API
v3 or belong to a later API generation.

## 1. Atomic native mutation transactions

Patch and inline-hook calls are currently separate operations. A plugin can
build its own preflight and rollback, but it cannot atomically coordinate
executable writes with loader ownership records, hook trampolines, and other
tracked owners.

A loader-owned transaction should stage supported patches and hooks, validate
all expected bytes and overlaps before the first write, prepare hook resources,
then commit everything or leave memory and ownership unchanged. Failure should
identify the operation, exact range, reason, and known owners. Successful
mutations should have an explicit unload rollback contract.

## 2. Typed item interaction events

Input actions describe bindings, while tooltip listeners describe text.
Neither reports a semantic interaction with one specific item. Modified-click
features still require private mouse handling, hovered-item discovery, or
stock-UI call-site interception.

An ordered event should provide a generation-aware ItemHandle, originating UI
surface, logical action or physical button, active modifiers, and interaction
phase. It should run on a documented UI boundary and return Continue or Consume.
Consumption should stop lower-priority listeners and normal handling.
Registrations should be removed automatically on unload, and the event should
not depend on a tooltip being built.

## 3. Transactions and moves for existing items

ItemServiceV1 can edit one existing item, while its transaction model consumes
inputs and creates outputs. It cannot atomically edit several existing items or
move an existing item between stock containers while preserving identity.

One authoritative transaction should support conservative batches of existing
item edits, partial stack debits, and moves between explicitly supported stock
containers. Before commit, the loader should revalidate handles, authority,
source, destination, placement, and every requested edit. Failure should change
nothing.

Success should preserve the item handle, seeds, socket contents, and untouched
state, then use the normal inventory, replication, and save paths. A first
version could support inventory, cube, equipment, cursor, and personal or shared
stash. Sensitive contexts such as trade, vendor, corpse, ground, and remote
access can remain unsupported until their ownership rules are proven.

## Common requirements

Each accepted boundary should remain optional through service queries, use
size-gated copied structures and safe handles, document thread and multiplayer
authority, clean up safely on unload, and fail without partial state.

Public contracts should ship only with working Loader/Core behavior, ABI tests,
a minimal example, unload coverage, and focused host/client validation. Existing
API v3 plugins must continue to load unchanged.

## Contribution offer

I am willing to implement the accepted public SDK contracts, ABI tests,
examples, and plugin migrations. I am also willing to collaborate on
Loader/Core implementation where the required source or integration access is
available.

## Maintainer questions

1. Which boundary is the smallest useful first implementation?
2. Should existing-item edits and stock-container moves share one transaction?
3. Can these remain optional API v3 service revisions, or does any boundary
   require a later API generation?
