#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "NsCheatsSettings.generated.h"

/** One key chord that runs a console command in a game world, for playtests. Not in Shipping builds */
USTRUCT(BlueprintType)
struct NSCHEATSRUNTIME_API FNsCheatHotkey
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Hotkey")
    FKey Key;

    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Hotkey")
    bool bCtrl = false;

    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Hotkey")
    bool bShift = false;

    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Hotkey")
    bool bAlt = false;

    /** The console command, for example ns.Cheats.Toggle */
    UPROPERTY(EditAnywhere, Config, BlueprintReadOnly, Category="Hotkey")
    FString Command;
};

/** The settings of NsCheats. A game sets its hotkeys in DefaultGame.ini */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Ns Cheats"))
class NSCHEATSRUNTIME_API UNsCheatsSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    /** Constructor */
    UNsCheatsSettings();

    /** The key text of the first hotkey that runs a command, for example "Ctrl+F3". Empty when no hotkey runs it */
    static FString GetHotkeyText(const FString& Command);

    /** Key chords that run console commands while a game world plays. They work even when a menu or a dialogue has the input */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Hotkeys")
    TArray<FNsCheatHotkey> Hotkeys;

    /** The folder for bug reports, under the Saved folder of the project */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Bug Report")
    FString ReportFolder;

    /** The number of log lines that a bug report keeps */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Bug Report", meta=(ClampMin="0"))
    int32 ReportLogLines;
};
