<!-- GH_ONLY_START -->
<h1 align="center">
  <br>
  <a href="https://github.com/mykaadev/NsCheats"><img src="https://raw.githubusercontent.com/mykaadev/NsCheats/refs/heads/main/Resources/Banner.png" alt="NsCheats" width="400"></a>
</h1>

<h4 align="center">A registry of debug cheats with a built-in window for Unreal Engine.</h4>

<div align="center">
    <a href="https://github.com/mykaadev/NsCheats/commits/main"><img src="https://img.shields.io/github/last-commit/mykaadev/NsCheats?style=plastic&logo=github&logoColor=white" alt="GitHub Last Commit"></a>
    <a href="https://github.com/mykaadev/NsCheats/issues"><img src="https://img.shields.io/github/issues-raw/mykaadev/NsCheats?style=plastic&logo=github&logoColor=white" alt="GitHub Issues"></a>
    <a href="https://github.com/mykaadev/NsCheats/pulls"><img src="https://img.shields.io/github/issues-pr-raw/mykaadev/NsCheats?style=plastic&logo=github&logoColor=white" alt="GitHub Pull Requests"></a>
    <a href="https://github.com/mykaadev/NsCheats"><img src="https://img.shields.io/github/stars/mykaadev/NsCheats?style=plastic&logo=github" alt="GitHub Stars"></a>
    <a href="https://twitter.com/mykaadev/"><img src="https://img.shields.io/twitter/follow/mykaadev?style=plastic&logo=x" alt="Twitter Follow"></a>

<p style="display:none;">
  <a href="#-summary">👀 Summary</a> •
  <a href="#-features">📦 Features</a> •
  <a href="#-requirements">⚙️ Requirements</a> •
  <a href="#-installation">🛠️ Installation</a> •
  <a href="#-getting-started">🚀 Getting Started</a> •
  <a href="#-entry-types">🧩 Entry Types</a> •
  <a href="#-api">🔧 API</a> •
  <a href="#-limits">🚧 Limits</a> •
  <a href="#-documentation">📚 Documentation</a> •
  <a href="#-credits">❤️ Credits</a> •
  <a href="#-support">📞 Support</a> •
  <a href="#-license">📃 License</a>
</p>
<a href="https://buymeacoffee.com/mykaadev"><img src="https://www.svgrepo.com/show/476855/coffee-to-go.svg" alt="Coffee" width="50px"></a>
<p><b>Buy me a coffee!</b></p>
</div>
<!-- GH_ONLY_END -->

## 👀 Summary
NsCheats is a registry of debug cheats with a built-in window. Game systems register buttons, toggles, number fields, text fields, pickers, and debug text. A developer opens the window during play and uses the cheats.

NsCheats is for development builds. In a Shipping build, the registry, the window, and the console commands compile out. NsCheats does not replicate, and it does not give an in-game UMG widget.

## 📦 Features
- **Seven entry types**: Buttons, toggles, number fields, text fields, pickers, debug text, and button rows.
- **C++ builder**: Register a full section of cheats in one call chain.
- **Blueprint support**: Register entries, and implement the provider interface, in Blueprint.
- **Provider discovery**: At start, the subsystem registers the game instance and world subsystems that implement the provider interface and already exist.
- **Tabs, sections, and search**: The window sorts entries into tabs and sections, with a search box.
- **Owner cleanup**: Entries of a destroyed owner leave the registry.
- **Console commands**: Open or close the window, log the registry, or run a console command.
- **Built-in cheats**: Log the registry, log the player location, and toggle `stat fps` and `stat unit`.

## ⚙️ Requirements
Unreal Engine 5.7+

## 🛠️ Installation
1. **Clone** or download this repository.
2. Copy the `NsCheats` folder into the `Plugins` folder of your project.
3. For C++, add `"NsCheatsRuntime"` to the module dependencies in `Build.cs`.
4. Generate the project files and enable the plugin.

## 🚀 Getting Started
Implement `INsCheatProviderInterface` on the object that owns the cheats:

```cpp
#include "NsCheatsSubsystem.h"

void UMyEconomySubsystem::RegisterCheats_Implementation(UNsCheatsSubsystem* CheatsSubsystem)
{
    CheatsSubsystem->ForProvider(this, TEXT("Economy"))
        .Section(TEXT("Coins"))
        .Button(TEXT("Economy.AddCoins"), INVTEXT("Add 1000 coins"),
            FNsCheatExecuteDelegate::CreateUObject(this, &UMyEconomySubsystem::AddDebugCoins))
        .Toggle(TEXT("Economy.FreePlay"), INVTEXT("Free play"),
            FNsCheatGetBoolDelegate::CreateLambda([this]() { return bFreePlay; }),
            FNsCheatSetBoolDelegate::CreateLambda([this](bool bValue) { bFreePlay = bValue; }));
}
```

Then call `RegisterProvider(this)` on the `UNsCheatsSubsystem` of the game instance. Do this in `BeginPlay` for an actor or a component, and call `UnregisterProvider(this)` in `EndPlay`.

Run `ns.Cheats.Toggle` in the console to open the window.

## 🧩 Entry Types
| Builder call | Blueprint call | Row in the window |
|---|---|---|
| `Button` | `Register Button Cheat` | A **Run** button |
| `Toggle` | `Register Toggle Cheat` | A check box |
| `Float` | `Register Float Cheat` | A number field |
| `Text` | `Register Text Cheat` | A text field, with a **Run** button if an action is bound |
| `Picker` | `Register Picker Cheat` | A dropdown, with a **Run** button if an action is bound |
| `DebugText` | `Register Debug Text Cheat` | Read-only text |
| `ButtonRow` | None | A row of buttons |

## 🔧 API
- `UNsCheatsSubsystem` – The game instance subsystem that owns the entries, the providers, the console commands, and the window.
- `INsCheatProviderInterface` – One function, `RegisterCheats(UNsCheatsSubsystem*)`, for C++ and Blueprint.
- `FNsCheatBuilder` – The C++ builder that `ForProvider(Owner, Tab)` returns.
- `FNsCheatEntry` – One entry: id, label, description, tab, section, sort order, type, owner, and delegates.
- `RegisterProvider` / `UnregisterProvider` – Add or remove a provider and its entries.
- `ExecuteEntry(Id)` – Run the action of an entry from code.
- `OpenCheatUI` / `CloseCheatUI` / `ToggleCheatUI` – Show or hide the window.
- `DumpRegisteredCheats` – Log all entries and return the list as a string.

## 🚧 Limits
- The window is a separate Slate window, not a widget in the game viewport.
- The plugin does not replicate. A cheat runs on the machine where you use it.
- `ButtonRow` has no Blueprint registration function.
- Automatic discovery runs one time, at start. Other providers must call `RegisterProvider`.

## 📚 Documentation
- [User Guide](https://mykaadev.github.io/plugins/nscheats/user-guide/) – Setup, common tasks, settings, and debug tools.
- [Technical Documentation](https://mykaadev.github.io/plugins/nscheats/tech-documentation/) – Architecture, lifetime, and extension points.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsCheats/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsCheats"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
