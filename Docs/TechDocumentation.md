# NsCheats Technical Documentation

## Architecture

### Modules

| Module | Type | Loading phase | Content |
|---|---|---|---|
| `NsCheatsRuntime` | Runtime | Default | The subsystem, the builder, the provider interface, the entry types, and the Slate window. |

The plugin has no content (`CanContainContent` is `false`). The module class `FNsCheatsRuntimeModule` has empty `StartupModule` and `ShutdownModule` functions. `NsCheatsRuntime.Build.cs` adds the public definition `NSCHEATS_API=NSCHEATSRUNTIME_API`. The headers use `NSCHEATSRUNTIME_API`.

### Folders

| Folder | Files |
|---|---|
| `Source/NsCheatsRuntime/Public` | `NsCheatsRuntime.h`, `NsCheatsSubsystem.h`, `NsCheatProviderInterface.h`, `NsCheatTypes.h` |
| `Source/NsCheatsRuntime/Private` | `NsCheatsRuntime.cpp`, `NsCheatsSubsystem.cpp` |

### Main Classes

| Type | Responsibility |
|---|---|
| `UNsCheatsSubsystem` | A `UGameInstanceSubsystem`. It owns the entry list, the provider list, the console commands, and the window. It builds all Slate widgets. |
| `FNsCheatBuilder` | A C++ builder that `ForProvider(Owner, Tab)` returns. It holds the subsystem, the owner, the tab, and the current section. Each call makes one `FNsCheatEntry` and sends it to `RegisterCheatEntry`. |
| `INsCheatProviderInterface` | A `BlueprintType` interface with one `BlueprintNativeEvent`, `RegisterCheats(UNsCheatsSubsystem*)`. |
| `FNsCheatEntry` | One entry: `Id`, `DisplayName`, `Description`, `Tab` (default `Debug`), `Category` (default `General`), `SortOrder`, `EntryType`, a weak `Owner`, a native delegate set, a dynamic delegate set, and `InlineActions`. |
| `ENsCheatEntryType` | `Button`, `Modifier` (float field), `DebugView` (debug text), `Toggle`, `Picker`, `Text`, `ButtonRow`. |
| `FNsCheatPickerOption` | `Value` (`FName`), `Label`, `Description`. The window shows `Label` and does not show `Description`. |
| `FNsCheatInlineAction` | One button of a `ButtonRow`: `Id`, `Label`, `Description` (the tooltip), `ExecuteDelegate`. It is a plain C++ struct. |

The delegates of `FNsCheatEntry` are not `UPROPERTY` members. Only the fields up to `Owner` are visible to reflection.

### Data Flow

Registration:

1. A provider calls a builder function or a `Register...Cheat` function.
2. `RegisterCheatEntry` ignores an entry with an empty `Id` and logs a warning.
3. It removes any entry with the same `Id` and adds the new entry.
4. It sorts the full list by tab, then category, then `SortOrder`.
5. It calls `RefreshCheatUI`, which builds the window content again if the window is open.

The builder replaces an empty tab with `Debug` and an empty section with `General`. The `Register...Cheat` functions copy `Tab` and `Category` as they are.

Delegate choice: every value read, value write, and action first tries the native delegate. If the native delegate is bound, only the native delegate runs. Else the dynamic (Blueprint) delegate runs.

Window build (`BuildCheatUIContent`):

1. `CleanInvalidEntries` removes entries whose owner is stale, and stale providers.
2. `NormalizeSelection` picks the first visible tab and section if the current ones are gone.
3. The window makes a search box, a **Reload** button, a tab bar, a section list (190 px wide), and the entry panel.
4. The entry panel sorts the entries of the selected tab and section by `SortOrder`, then by the label text.

Live values: the float field, the check box, and the text field read their values through Slate attribute lambdas, so they update every Slate paint. The debug text and the picker selection are read one time, when the row is built. A button click, a value commit, a selection change, a search edit, and **Reload** build the window content again.

Threading: all work runs on the game thread. The plugin starts no tasks or threads.

## Lifetime

1. The game instance creates `UNsCheatsSubsystem`. The class does not override `ShouldCreateSubsystem`, so it exists in all builds and all game instances.
2. `Initialize` (not in Shipping):
   1. `RegisterBuiltInCheats` adds four entries in the `Debug` and `Performance` tabs.
   2. `AutoRegisterSubsystemProviders` calls `RegisterProvider` on each game instance subsystem in the collection at that time, except itself. Then it does the same for each world subsystem of `GetWorld()`, if there is a world.
   3. `RegisterConsoleCommands` registers the `ns.Cheats.*` commands.
3. `RegisterProvider` ignores a null object, an object that does not implement `INsCheatProviderInterface`, and an object it already has. Else it stores a weak pointer, calls `Execute_RegisterCheats`, and refreshes the window.
4. Entries with a destroyed owner stay in the list until the next `CleanInvalidEntries`. `ExecuteEntry`, `DumpRegisteredCheats`, and each window build call it. An entry with no owner is never stale.
5. `Deinitialize` (not in Shipping) closes the window and unregisters the console commands that this instance registered.

Game instance subsystems start in an order that the engine sets. Discovery only sees the subsystems that started before `UNsCheatsSubsystem`. A provider subsystem that must be found calls `Collection.InitializeDependency<UNsCheatsSubsystem>()` and then `RegisterProvider(this)`.

Console commands are global. `RegisterConsoleCommands` registers a command only if no console object has that name. With more than one game instance in PIE, the first subsystem registers the commands, and the commands call that subsystem. Only the subsystem that registered a command unregisters it.

The window is an `SWindow` that `FSlateApplication::AddWindow` adds. `CheatWindow` is a weak pointer, so the subsystem does not keep a closed window alive. `CloseCheatUI` calls `RequestDestroyWindow`.

### Shipping

`UE_BUILD_SHIPPING` removes the bodies of `Initialize`, `Deinitialize`, `RegisterProvider`, `UnregisterProvider`, `RegisterCheatEntry`, the unregister and clean functions, `ExecuteEntry`, and the window functions. In Shipping:

- The `Register...Cheat` functions and the builder still make an entry, but `RegisterCheatEntry` drops it.
- `ExecuteEntry` returns `false`.
- `DumpRegisteredCheats` logs and returns only the header line.
- No console commands exist.

Provider code needs no `#if` guards to compile in Shipping.

## Extension Points

### Provider interface

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="NsCheats")
void RegisterCheats(UNsCheatsSubsystem* CheatsSubsystem);
```

C++ classes override `RegisterCheats_Implementation`. Blueprint classes implement the `Register Cheats` event.

### Builder calls

| Call | Entry type | Delegates |
|---|---|---|
| `Section(FName)` | None | Sets the section of the next entries. |
| `Button(Id, Label, Execute, Description, SortOrder)` | `Button` | `FNsCheatExecuteDelegate` |
| `Toggle(Id, Label, GetValue, SetValue, Description, SortOrder)` | `Toggle` | `FNsCheatGetBoolDelegate`, `FNsCheatSetBoolDelegate` |
| `DebugText(Id, Label, DebugText, Description, SortOrder)` | `DebugView` | `FNsCheatDebugTextDelegate` |
| `Float(Id, Label, GetValue, SetValue, Description, SortOrder)` | `Modifier` | `FNsCheatGetFloatDelegate`, `FNsCheatSetFloatDelegate` |
| `Text(Id, Label, GetValue, SetValue, Execute, Description, SortOrder)` | `Text` | `FNsCheatGetStringDelegate`, `FNsCheatSetStringDelegate`, optional `FNsCheatExecuteDelegate` |
| `Picker(Id, Label, GetOptions, GetValue, SetValue, Refresh, Description, SortOrder)` | `Picker` | `FNsCheatGetPickerOptionsDelegate`, `FNsCheatGetPickerValueDelegate`, `FNsCheatSetPickerValueDelegate`, optional `FNsCheatExecuteDelegate` |
| `ButtonRow(Id, Label, Actions, Description, SortOrder)` | `ButtonRow` | `TArray<FNsCheatInlineAction>` |

`Description` defaults to empty text and `SortOrder` to `0`.

### Blueprint functions

`RegisterButtonCheat`, `RegisterToggleCheat`, `RegisterDebugTextCheat`, `RegisterFloatCheat`, `RegisterTextCheat`, and `RegisterPickerCheat` take `Id`, `Tab`, `Category`, `DisplayName`, `Description`, `Owner`, the dynamic delegates (`FNsCheatDynamic...Delegate`), and `SortOrder = 0`. There is no Blueprint function for `ButtonRow`.

### Direct entries

`RegisterCheatEntry(const FNsCheatEntry&)` is public C++. Use it to register an entry that you build by hand. `GetEntries()` returns the current list.

### New entry types

The window picks a widget in `BuildCheatEntryWidget`: `Picker`, `Text`, and `ButtonRow` have their own build functions. All other types use one generic row that shows debug text, a float field, a check box, or a **Run** button, based on which delegates are bound. A new type needs a builder call, a value in `ENsCheatEntryType`, and a branch in `BuildCheatEntryWidget`.

## Dependencies

| Dependency | Kind | Why |
|---|---|---|
| `Core` | Public module | Core types. |
| `CoreUObject` | Public module | `UObject`, interfaces, and reflection. |
| `Engine` | Public module | `UGameInstanceSubsystem`, `UWorldSubsystem`, `GEngine->Exec`. |
| `Slate` | Private module | `SWindow`, buttons, and the other window widgets. |
| `SlateCore` | Private module | Slate base types and `FCoreStyle` fonts. |
| `InputCore` | Private module | Listed in `Build.cs`. The source includes no `InputCore` header. |
| `UMG` | Private module | Listed in `Build.cs`. The source includes no `UMG` header. |

The `.uplugin` lists no plugin dependencies.

## Performance

- Each registration removes by `Id`, adds, and sorts the full entry list. If the window is open, it also builds the full window content again.
- Each window build copies the visible entries into a new array and makes a new widget for each row.
- `GetVisibleTabs` and `GetVisibleSectionsForTab` scan all entries, and the search check formats one string for each entry.
- The float field, check box, and text field lambdas search the entry list by `Id` on each read.
- The plugin has no tick and no timer. Nothing runs while the window is closed, except registration calls.

## Networking

The plugin does not replicate. Entries, delegates, and the window are local to one game instance.

## Testing

The plugin has no automated tests.

Manual check with the built-in entries:

1. Start PIE in a Development build and run `ns.Cheats.Toggle`. Make sure that the window shows the `Debug` and `Performance` tabs.
2. In `Performance` > `Stats`, click **Run** on `Toggle stat fps`. Make sure that the FPS stat shows.
3. Run `ns.Cheats.Dump`. Make sure that the log shows the built-in entries under `LogNsCheats`.
4. Type `stat` in the search box. Make sure that the `Stats` section shows `Toggle stat fps` and `Toggle stat unit`.

## Change Rules

- Keep the `ns.Cheats.*` command names. Developers and scripts call them by name.
- Keep the `#if !UE_BUILD_SHIPPING` guards on new public functions that touch the registry or the window.
- Keep the names of the `Register...Cheat` parameters and of the `RegisterCheats` parameter. Blueprint pins use these names, and a rename breaks the pin links.
- Keep the `INsCheatProviderInterface` and `UNsCheatsSubsystem` class names. Blueprints that implement or call them store the class names.
