# NsCheats User Guide

## Purpose

NsCheats is a registry of debug cheats with a built-in window. Game systems register buttons, toggles, values, text fields, pickers, and debug text. Developers open the window and use them during play.

NsCheats is for development builds only. In a shipping build, every function does nothing. Do not use it for player-facing features or settings.

## Setup

1. Add `NsCheats` to the `Plugins` list of the `.uproject` file.
2. For C++, add `"NsCheatsRuntime"` to the module dependencies in `Build.cs`.

The plugin adds `UNsCheatsSubsystem`, a game instance subsystem. It has no project settings.

## Quick Start

### 1. Make a provider

Implement `INsCheatProviderInterface` on a game instance subsystem, a world subsystem, an actor, or a component.

```cpp
void UMyEconomySubsystem::RegisterCheats_Implementation(UNsCheatsSubsystem* const CheatsSubsystem)
{
    CheatsSubsystem->ForProvider(this, TEXT("Economy"))
        .Section(TEXT("Chips"))
        .Button(TEXT("Economy.AddChips"), INVTEXT("Add 1000 chips"), FNsCheatExecuteDelegate::CreateUObject(this, &UMyEconomySubsystem::AddDebugChips))
        .Toggle(TEXT("Economy.FreePlay"), INVTEXT("Free play"),
            FNsCheatGetBoolDelegate::CreateLambda([this]() { return bFreePlay; }),
            FNsCheatSetBoolDelegate::CreateLambda([this](const bool bValue) { bFreePlay = bValue; }));
}
```

### 2. Register the provider

- Game instance subsystems are found automatically.
- World subsystems that exist when `UNsCheatsSubsystem` starts are found automatically.
- All other objects call `RegisterProvider(this)`, for example in `BeginPlay`. Call `UnregisterProvider(this)` in `EndPlay`.

### 3. Open the window

Run `ns.Cheats.Toggle` in the console. Bind it to a debug key if needed.

## Common Tasks

### Entry types

| Builder call | Blueprint call | Row in the window |
|---|---|---|
| `Button` | `Register Button Cheat` | A button. |
| `Toggle` | `Register Toggle Cheat` | A check box. |
| `Float` | `Register Float Cheat` | A number field. |
| `Text` | `Register Text Cheat` | A text field with an optional run button. |
| `Picker` | `Register Picker Cheat` | A dropdown with an optional refresh. |
| `DebugText` | `Register Debug Text Cheat` | Read-only text. |
| `ButtonRow` | None | A row of small buttons. |

### Organize entries

- `Tab` sets the tab. The default is `Debug`.
- `Section` sets the section in the tab. The default is `General`.
- `SortOrder` sets the order in a section.
- The window has a search box.

### Run a cheat from code

Call `ExecuteEntry(Id)`.

### Run any console command

Run `ns.Cheats.Exec <command>`, for example `ns.Cheats.Exec stat fps`.

## Rules

- Give each entry a unique `Id`. A new entry with the same `Id` replaces the old one. Prefix ids with the system name, for example `Economy.AddChips`.
- Pass the provider as the owner. Entries of a destroyed owner are removed.
- Unregister actors and components when they end play.
- Do not put shipping logic in a cheat delegate. The delegate never runs in shipping.

## Settings

NsCheats has no settings.

## Debug

| Command | Effect |
|---|---|
| `ns.Cheats.Open` | Opens the window. |
| `ns.Cheats.Close` | Closes the window. |
| `ns.Cheats.Toggle` | Opens or closes the window. |
| `ns.Cheats.Dump` | Logs all entries. |
| `ns.Cheats.Exec <command>` | Runs a console command. |

Built-in entries: dump the registry, log the player location, toggle `stat fps`, and toggle `stat unit`.

Log category: `LogNsCheats`.

## Limits

- The window is a separate Slate window, not an in-game widget. It does not use `NsUIFramework` and does not work with a gamepad.
- World subsystems made after `UNsCheatsSubsystem` starts are not found automatically. Register them with `RegisterProvider`.
- No networking. A cheat runs on the machine where it is clicked. A server cheat needs a server RPC in the game.
- `ButtonRow` has no Blueprint registration function.
