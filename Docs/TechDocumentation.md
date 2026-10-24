# NsCheats Technical Documentation

## Architecture

One runtime module, `NsCheatsRuntime`. The build file defines `NSCHEATS_API` as `NSCHEATSRUNTIME_API`.

| Type | Role |
|---|---|
| `UNsCheatsSubsystem` | Game instance subsystem. It owns the entries, the providers, the console commands, and the window. |
| `INsCheatProviderInterface` | Interface with `RegisterCheats(UNsCheatsSubsystem*)`. |
| `FNsCheatBuilder` | Fluent C++ builder from `ForProvider(Owner, Tab)`. |
| `FNsCheatEntry` | One entry: id, labels, tab, category, sort order, type, weak owner, and native and dynamic delegates. |
| `ENsCheatEntryType` | `Button`, `Modifier` (float), `DebugView` (text), `Toggle`, `Picker`, `Text`, `ButtonRow`. |
| `FNsCheatPickerOption` | `Value`, `Label`, `Description`. |
| `FNsCheatInlineAction` | One button in a `ButtonRow`. Native only. |

Each entry holds a native delegate set for C++ and a dynamic delegate set for Blueprint. The native delegate runs first when both are bound.

### Registration flow

1. `RegisterCheatEntry` rejects an entry with an empty id.
2. It removes any entry with the same id, adds the new one, and sorts by tab, category, and sort order.
3. It rebuilds the window content if the window is open.

## Lifetime

- `Initialize` registers the built-in entries, finds providers among game instance and world subsystems, and registers the console commands.
- `Deinitialize` closes the window and unregisters the console commands.
- Console commands are global. With more than one game instance in PIE, only the first subsystem registers and owns them.
- `CleanInvalidEntries` removes entries with a stale owner and stale providers. `ExecuteEntry` and `DumpRegisteredCheats` call it.
- In shipping (`UE_BUILD_SHIPPING`), the bodies of all public functions compile out.

## Extension Points

| Point | Use |
|---|---|
| `INsCheatProviderInterface` | Implement in C++ or Blueprint. |
| `FNsCheatBuilder` | Add a new builder call for a new entry type. |
| `ENsCheatEntryType` | Add a type, then add a widget in `BuildCheatEntryWidget`. |

## Dependencies

Public: `Core`, `CoreUObject`, `Engine`. Private: `InputCore`, `Slate`, `SlateCore`, `UMG`. No other studio plugin.

## Performance

- Each registration sorts the full entry list and rebuilds the open window. Register in batches at startup, not every frame.
- The window rebuilds all rows on each change. This is fine for hundreds of entries.
- Debug text and values update through Slate attributes while the window is open.

## Networking

None. The registry and the window are local.

## Testing

NsCheats has no automated tests. Manual test path:

1. Run `ns.Cheats.Toggle`. Make sure that the window opens with the `Debug` and `Performance` tabs.
2. Click "Toggle stat fps". Make sure that the stat shows.
3. Register a provider actor. Destroy it. Run `ns.Cheats.Dump`. Make sure that its entries are gone.
4. Build a shipping target. Make sure that the build has no cheat window or commands.

## Change Rules

- Keep the `ns.Cheats.*` command names stable. Developers and scripts use them.
- Keep the shipping guards on every new public function.
- An in-game window that uses `NsUIFramework` is a candidate feature. Put it in an optional integration module, so that the core plugin keeps no dependency on `NsUIFramework`.
