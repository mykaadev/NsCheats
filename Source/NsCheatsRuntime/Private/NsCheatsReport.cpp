// The hotkeys and the bug report of NsCheats. Not in Shipping builds
#include "NsCheatsSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformOutputDevices.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Misc/Paths.h"
#include "NsCheatsSettings.h"
#include "UnrealClient.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogNsCheatsReport, Log, All);

#if !UE_BUILD_SHIPPING
/** Runs the hotkeys of the settings before the game, the menus, or a dialogue get the key */
class FNsCheatsHotkeyProcessor : public IInputProcessor
{
public:
    explicit FNsCheatsHotkeyProcessor(UNsCheatsSubsystem* const InOwner)
        : Owner(InOwner)
    {
    }

    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
    {
    }

    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
    {
        UNsCheatsSubsystem* const Subsystem = Owner.Get();
        if (Subsystem == nullptr || InKeyEvent.IsRepeat() || !Subsystem->IsOwnWindowActive())
        {
            return false;
        }

        for (const FNsCheatHotkey& Hotkey : GetDefault<UNsCheatsSettings>()->Hotkeys)
        {
            if (Hotkey.Key == InKeyEvent.GetKey()
                && Hotkey.bCtrl == InKeyEvent.IsControlDown()
                && Hotkey.bShift == InKeyEvent.IsShiftDown()
                && Hotkey.bAlt == InKeyEvent.IsAltDown())
            {
                Subsystem->RunHotkeyCommand(Hotkey.Command);
                return true;
            }
        }
        return false;
    }

    virtual const TCHAR* GetDebugName() const override
    {
        return TEXT("NsCheatsHotkeys");
    }

private:
    TWeakObjectPtr<UNsCheatsSubsystem> Owner;
};

FString UNsCheatsSubsystem::ReadLogTail(const int32 LineCount)
{
    if (LineCount <= 0)
    {
        return FString();
    }

    GLog->Flush();
    const FString LogFilePath = FPlatformOutputDevices::GetAbsoluteLogFilename();
    TUniquePtr<FArchive> Reader(IFileManager::Get().CreateFileReader(*LogFilePath, FILEREAD_AllowWrite));
    if (!Reader.IsValid())
    {
        return FString::Printf(TEXT("(The log %s cannot be read.)"), *LogFilePath);
    }

    // Read only the end of the file: a long play session writes a big log
    const int64 Size = Reader->TotalSize();
    const int64 Start = FMath::Max<int64>(0, Size - 1024 * 1024);
    TArray<uint8> Bytes;
    Bytes.SetNumUninitialized(static_cast<int32>(Size - Start));
    Reader->Seek(Start);
    Reader->Serialize(Bytes.GetData(), Bytes.Num());

    FString Text;
    FFileHelper::BufferToString(Text, Bytes.GetData(), Bytes.Num());
    TArray<FString> Lines;
    Text.ParseIntoArrayLines(Lines, false);
    const int32 First = FMath::Max(0, Lines.Num() - LineCount);
    return FString::Join(TArrayView<const FString>(Lines.GetData() + First, Lines.Num() - First), TEXT("\n"));
}
#endif

void UNsCheatsSubsystem::RegisterHotkeys()
{
#if !UE_BUILD_SHIPPING
    if (FSlateApplication::IsInitialized() && !HotkeyProcessor.IsValid())
    {
        HotkeyProcessor = MakeShared<FNsCheatsHotkeyProcessor>(this);
        FSlateApplication::Get().RegisterInputPreProcessor(HotkeyProcessor);
    }
#endif
}

void UNsCheatsSubsystem::UnregisterHotkeys()
{
#if !UE_BUILD_SHIPPING
    if (HotkeyProcessor.IsValid() && FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(HotkeyProcessor);
    }
    HotkeyProcessor.Reset();
#endif
}

bool UNsCheatsSubsystem::IsOwnWindowActive() const
{
#if !UE_BUILD_SHIPPING
    // A key in an editor panel, or in the window of another play client, belongs there
    const TSharedPtr<SWindow> Active = FSlateApplication::IsInitialized() ? FSlateApplication::Get().GetActiveTopLevelWindow() : nullptr;
    UGameViewportClient* const Viewport = GetGameInstance() != nullptr ? GetGameInstance()->GetGameViewportClient() : nullptr;
    const TSharedPtr<SWindow> GameWindow = Viewport != nullptr ? Viewport->GetWindow() : nullptr;
    return Active.IsValid() && (Active == GameWindow || Active == CheatWindow.Pin());
#else
    return false;
#endif
}

void UNsCheatsSubsystem::RunHotkeyCommand(const FString& Command)
{
#if !UE_BUILD_SHIPPING
    UWorld* const World = GetWorld();
    if (GEngine == nullptr || World == nullptr || Command.IsEmpty())
    {
        return;
    }

    UE_LOG(LogNsCheatsReport, Log, TEXT("Hotkey runs: %s"), *Command);
    GEngine->Exec(World, *Command);
#endif
}

FString UNsCheatsSubsystem::SaveReport()
{
#if !UE_BUILD_SHIPPING
    const UNsCheatsSettings* const Settings = GetDefault<UNsCheatsSettings>();
    const FString Stamp = FDateTime::Now().ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
    const FString Folder = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / Settings->ReportFolder / Stamp);
    IFileManager::Get().MakeDirectory(*Folder, true);

    // The screenshot comes with the next frame, so it shows the game as the person saw it, with its UI
    FScreenshotRequest::RequestScreenshot(Folder / TEXT("screenshot.png"), true, false);

    const UWorld* const World = GetWorld();
    const APlayerController* const PlayerController = World != nullptr ? World->GetFirstPlayerController() : nullptr;
    const APawn* const Pawn = PlayerController != nullptr ? PlayerController->GetPawn() : nullptr;

    FString Report = FString::Printf(TEXT("Bug report %s\n\nMap: %s\nGame time: %.1f s\n"), *Stamp, World != nullptr ? *World->GetMapName() : TEXT("(none)"), World != nullptr ? World->GetTimeSeconds() : 0.f);
    Report += Pawn != nullptr
        ? FString::Printf(TEXT("Player: %s at %s, facing %s\n"), *Pawn->GetName(), *Pawn->GetActorLocation().ToCompactString(), *Pawn->GetActorRotation().ToCompactString())
        : FString(TEXT("Player: no pawn\n"));

    // Every debug text of the cheat window: each provider shows its state there, so the report gets it too
    for (const FNsCheatEntry& Entry : Entries)
    {
        if (Entry.EntryType == ENsCheatEntryType::DebugView)
        {
            Report += FString::Printf(TEXT("\n== %s / %s / %s ==\n%s\n"), *Entry.Tab.ToString(), *Entry.Category.ToString(), *Entry.DisplayName.ToString(), *GetEntryDebugText(Entry).ToString());
        }
    }

    FFileHelper::SaveStringToFile(Report, *(Folder / TEXT("report.txt")), FFileHelper::EEncodingOptions::ForceUTF8);
    FFileHelper::SaveStringToFile(ReadLogTail(Settings->ReportLogLines), *(Folder / TEXT("log.txt")), FFileHelper::EEncodingOptions::ForceUTF8);
    FPlatformApplicationMisc::ClipboardCopy(*Folder);
    UE_LOG(LogNsCheatsReport, Log, TEXT("Bug report saved to %s. The path is in the clipboard."), *Folder);
    return Folder;
#else
    return FString();
#endif
}

void UNsCheatsSubsystem::OpenReportWindow()
{
#if !UE_BUILD_SHIPPING
    const FString Folder = SaveReport();
    if (!FSlateApplication::IsInitialized() || Folder.IsEmpty())
    {
        return;
    }
    if (TSharedPtr<SWindow> OldWindow = ReportWindow.Pin())
    {
        OldWindow->RequestDestroyWindow();
    }

    TSharedPtr<SMultiLineEditableTextBox> NoteBox;
    const TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString(TEXT("Bug report")))
        .ClientSize(FVector2D(560.f, 320.f))
        .SupportsMinimize(false)
        .SupportsMaximize(false);

    const TWeakPtr<SWindow> WeakWindow = Window;
    Window->SetContent(
        SNew(SBox)
        .Padding(12.f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(0.f, 0.f, 0.f, 8.f)
            [
                SNew(STextBlock)
                .AutoWrapText(true)
                .Text(FText::FromString(FString::Printf(TEXT("Saved: the screenshot, the game state, and the log.\n%s\nThe path is in the clipboard. Write what went wrong, then save the note."), *Folder)))
            ]
            + SVerticalBox::Slot().FillHeight(1.f)
            [
                SAssignNew(NoteBox, SMultiLineEditableTextBox)
                .HintText(FText::FromString(TEXT("What did you do, what did you expect, what happened?")))
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Right).Padding(0.f, 8.f, 0.f, 0.f)
            [
                SNew(SButton)
                .Text(FText::FromString(TEXT("Save the note")))
                .OnClicked_Lambda([NoteBox, Folder, WeakWindow]()
                {
                    if (NoteBox.IsValid() && !NoteBox->GetText().IsEmpty())
                    {
                        FFileHelper::SaveStringToFile(NoteBox->GetText().ToString(), *(Folder / TEXT("note.txt")), FFileHelper::EEncodingOptions::ForceUTF8);
                        UE_LOG(LogNsCheatsReport, Log, TEXT("Bug report note saved to %s."), *Folder);
                    }
                    if (const TSharedPtr<SWindow> Pinned = WeakWindow.Pin())
                    {
                        Pinned->RequestDestroyWindow();
                    }
                    return FReply::Handled();
                })
            ]
        ]);

    FSlateApplication::Get().AddWindow(Window);
    ReportWindow = Window;
    FSlateApplication::Get().SetKeyboardFocus(NoteBox);
#endif
}
