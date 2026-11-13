#include "NsCheatsSettings.h"

UNsCheatsSettings::UNsCheatsSettings()
    : ReportFolder(TEXT("QA"))
    , ReportLogLines(300)
    , PickMarkerColor(FColor(255, 200, 0))
    , PickMarkerRadius(15.f)
    , PickTraceDistance(100000.f)
{
}

FString UNsCheatsSettings::GetHotkeyText(const FString& Command)
{
    for (const FNsCheatHotkey& Hotkey : GetDefault<UNsCheatsSettings>()->Hotkeys)
    {
        if (Hotkey.Command.Equals(Command, ESearchCase::IgnoreCase) && Hotkey.Key.IsValid())
        {
            FString Text;
            Text += Hotkey.bCtrl ? TEXT("Ctrl+") : TEXT("");
            Text += Hotkey.bShift ? TEXT("Shift+") : TEXT("");
            Text += Hotkey.bAlt ? TEXT("Alt+") : TEXT("");
            return Text + Hotkey.Key.GetDisplayName(false).ToString();
        }
    }
    return FString();
}
