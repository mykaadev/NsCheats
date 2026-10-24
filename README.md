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
NsCheats is a registry of debug cheats with a built-in window. Game systems register buttons, toggles, values, text fields, pickers, and debug text. Developers open the window and use them during play.

NsCheats is for development builds only. In a shipping build, every function does nothing.

## 📦 Features
- **Many entry types**: Buttons, toggles, number fields, text fields, pickers, debug text, and button rows.
- **Fluent C++ builder**: Register a full section of cheats in one chain.
- **Blueprint support**: Register entries and implement the provider interface in Blueprint.
- **Automatic providers**: Game instance subsystems and world subsystems register without extra code.
- **Tabs, sections, and search**: Keep a large set of cheats in order.
- **Console commands**: Open the window, dump the registry, or run any console command.
- **Shipping safe**: All public function bodies compile out in shipping builds.

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
void UMyEconomySubsystem::RegisterCheats_Implementation(UNsCheatsSubsystem* const CheatsSubsystem)
{
    CheatsSubsystem->ForProvider(this, TEXT("Economy"))
        .Section(TEXT("Coins"))
        .Button(TEXT("Economy.AddCoins"), INVTEXT("Add 1000 coins"),
            FNsCheatExecuteDelegate::CreateUObject(this, &UMyEconomySubsystem::AddDebugCoins))
        .Toggle(TEXT("Economy.FreePlay"), INVTEXT("Free play"),
            FNsCheatGetBoolDelegate::CreateLambda([this]() { return bFreePlay; }),
            FNsCheatSetBoolDelegate::CreateLambda([this](const bool bValue) { bFreePlay = bValue; }));
}
```

Game instance subsystems register automatically. Actors and components call `RegisterProvider(this)` in `BeginPlay` and `UnregisterProvider(this)` in `EndPlay`.

Run `ns.Cheats.Toggle` in the console to open the window.

## 🔧 API
### Key Classes
- `UNsCheatsSubsystem` – Game instance subsystem that owns the entries, the providers, the console commands, and the window.
- `INsCheatProviderInterface` – Interface with `RegisterCheats`, for C++ and Blueprint.
- `FNsCheatBuilder` – Fluent C++ builder from `ForProvider(Owner, Tab)`.
- `FNsCheatEntry` – One entry: id, labels, tab, section, sort order, type, owner, and delegates.

### Builder Calls
| Builder call | Blueprint call | Row in the window |
|---|---|---|
| `Button` | `Register Button Cheat` | A button |
| `Toggle` | `Register Toggle Cheat` | A check box |
| `Float` | `Register Float Cheat` | A number field |
| `Text` | `Register Text Cheat` | A text field with an optional run button |
| `Picker` | `Register Picker Cheat` | A dropdown with an optional refresh |
| `DebugText` | `Register Debug Text Cheat` | Read-only text |
| `ButtonRow` | None | A row of small buttons |

### Console Commands
| Command | Effect |
|---|---|
| `ns.Cheats.Open` | Opens the window |
| `ns.Cheats.Close` | Closes the window |
| `ns.Cheats.Toggle` | Opens or closes the window |
| `ns.Cheats.Dump` | Logs all entries |
| `ns.Cheats.Exec <command>` | Runs a console command |

## 🚧 Limits
- The window is a separate Slate window, not an in-game widget. It does not work with a gamepad.
- No networking. A cheat runs on the machine where you click it.
- `ButtonRow` has no Blueprint registration function.

## 📚 Documentation
- [User Guide](https://mykaadev.github.io/plugins/nscheats/user-guide/) – Setup, common tasks, settings, and debug tools.
- [Technical Documentation](https://mykaadev.github.io/plugins/nscheats/tech-documentation/) – Architecture, lifetime, extension points, and tests.

<!-- GH_ONLY_START -->
## ❤️ Credits
<a href="https://github.com/mykaadev/NsCheats/graphs/contributors"><img src="https://contrib.rocks/image?repo=mykaadev/NsCheats"/></a>

## 📞 Support
Reach out via the **[profile page](https://github.com/mykaadev)**.

## 📃 License
[![License](https://img.shields.io/badge/license-MIT-green)](https://www.tldrlegal.com/license/mit-license)
<!-- GH_ONLY_END -->
