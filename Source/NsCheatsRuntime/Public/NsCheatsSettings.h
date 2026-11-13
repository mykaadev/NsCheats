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

/** Where the cheat menu opens */
UENUM(BlueprintType)
enum class ENsCheatsDisplayMode : uint8
{
    /** A separate window next to the game */
    Window,
    /** A panel inside the game view. The game keeps running behind it */
    InGame
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

    /** Where the cheat menu opens: a separate window, or a panel inside the game view */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display")
    ENsCheatsDisplayMode DisplayMode;

    /** The width of the in-game panel */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display", meta=(ClampMin="200.0", EditCondition="DisplayMode == ENsCheatsDisplayMode::InGame"))
    float InGamePanelWidth;

    /** The space between the in-game panel and the edges of the game view, and inside the panel */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display", meta=(ClampMin="0.0", EditCondition="DisplayMode == ENsCheatsDisplayMode::InGame"))
    float InGamePanelMargin;

    /** The background color of the in-game panel. The alpha lets the game show through */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display", meta=(EditCondition="DisplayMode == ENsCheatsDisplayMode::InGame"))
    FLinearColor InGamePanelColor;

    /** The layer of the in-game panel in the game view. A higher layer covers the game widgets */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display", meta=(EditCondition="DisplayMode == ENsCheatsDisplayMode::InGame"))
    int32 InGamePanelZOrder;

    /** Pauses the game while the in-game panel is open */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Display", meta=(EditCondition="DisplayMode == ENsCheatsDisplayMode::InGame"))
    bool bPauseWhileOpen;

    /** The color of the marker of the point picker */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Point Picker")
    FColor PickMarkerColor;

    /** The radius of the marker of the point picker */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Point Picker", meta=(ClampMin="1.0", Units="cm"))
    float PickMarkerRadius;

    /** How far the point picker looks along the view center */
    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category="Point Picker", meta=(ClampMin="100.0", Units="cm"))
    float PickTraceDistance;
};
