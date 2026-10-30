#include "NsCheatsSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "NsCheatProviderInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogNsCheats, Log, All);

FNsCheatBuilder::FNsCheatBuilder(UNsCheatsSubsystem* InSubsystem, UObject* InOwner, const FName InTab)
    : Subsystem(InSubsystem)
    , Owner(InOwner)
    , Tab(InTab)
{
}

FNsCheatBuilder& FNsCheatBuilder::Section(const FName InSection)
{
    CurrentSection = InSection.IsNone() ? FName("General") : InSection;
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::Button(const FName Id, const FText Label, FNsCheatExecuteDelegate Execute, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::Button, SortOrder);
    Entry.ExecuteDelegate = MoveTemp(Execute);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::Toggle(const FName Id, const FText Label, FNsCheatGetBoolDelegate GetValue, FNsCheatSetBoolDelegate SetValue, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::Toggle, SortOrder);
    Entry.GetBoolDelegate = MoveTemp(GetValue);
    Entry.SetBoolDelegate = MoveTemp(SetValue);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::DebugText(const FName Id, const FText Label, FNsCheatDebugTextDelegate DebugText, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::DebugView, SortOrder);
    Entry.DebugTextDelegate = MoveTemp(DebugText);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::Float(const FName Id, const FText Label, FNsCheatGetFloatDelegate GetValue, FNsCheatSetFloatDelegate SetValue, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::Modifier, SortOrder);
    Entry.GetFloatDelegate = MoveTemp(GetValue);
    Entry.SetFloatDelegate = MoveTemp(SetValue);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::Text(const FName Id, const FText Label, FNsCheatGetStringDelegate GetValue, FNsCheatSetStringDelegate SetValue, FNsCheatExecuteDelegate Execute, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::Text, SortOrder);
    Entry.GetStringDelegate = MoveTemp(GetValue);
    Entry.SetStringDelegate = MoveTemp(SetValue);
    Entry.ExecuteDelegate = MoveTemp(Execute);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::Picker(const FName Id, const FText Label, FNsCheatGetPickerOptionsDelegate GetOptions, FNsCheatGetPickerValueDelegate GetValue, FNsCheatSetPickerValueDelegate SetValue, FNsCheatExecuteDelegate Refresh, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::Picker, SortOrder);
    Entry.GetPickerOptionsDelegate = MoveTemp(GetOptions);
    Entry.GetPickerValueDelegate = MoveTemp(GetValue);
    Entry.SetPickerValueDelegate = MoveTemp(SetValue);
    Entry.ExecuteDelegate = MoveTemp(Refresh);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatBuilder& FNsCheatBuilder::ButtonRow(const FName Id, const FText Label, TArray<FNsCheatInlineAction> Actions, const FText Description, const int32 SortOrder)
{
    FNsCheatEntry Entry = MakeEntry(Id, Label, Description, ENsCheatEntryType::ButtonRow, SortOrder);
    Entry.InlineActions = MoveTemp(Actions);
    Register(MoveTemp(Entry));
    return *this;
}

FNsCheatEntry FNsCheatBuilder::MakeEntry(const FName Id, const FText Label, const FText Description, const ENsCheatEntryType Type, const int32 SortOrder) const
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.DisplayName = Label;
    Entry.Description = Description;
    Entry.Tab = Tab.IsNone() ? FName("Debug") : Tab;
    Entry.Category = CurrentSection;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = Type;
    Entry.Owner = Owner;
    return Entry;
}

void FNsCheatBuilder::Register(FNsCheatEntry&& Entry) const
{
    if (Subsystem != nullptr)
    {
        Subsystem->RegisterCheatEntry(Entry);
    }
}

void UNsCheatsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if !UE_BUILD_SHIPPING
    RegisterBuiltInCheats();
    AutoRegisterSubsystemProviders();
    RegisterConsoleCommands();
#endif
}

void UNsCheatsSubsystem::Deinitialize()
{
#if !UE_BUILD_SHIPPING
    CloseCheatUI();
    UnregisterConsoleCommands();
#endif
    Super::Deinitialize();
}

FNsCheatBuilder UNsCheatsSubsystem::ForProvider(UObject* Owner, const FName Tab)
{
    return FNsCheatBuilder(this, Owner, Tab);
}

void UNsCheatsSubsystem::RegisterProvider(UObject* Provider)
{
#if !UE_BUILD_SHIPPING
    if (Provider == nullptr || !Provider->GetClass()->ImplementsInterface(UNsCheatProviderInterface::StaticClass()) || RegisteredProviders.Contains(Provider))
    {
        return;
    }

    RegisteredProviders.Add(Provider);
    INsCheatProviderInterface::Execute_RegisterCheats(Provider, this);
    RefreshCheatUI();
#endif
}

void UNsCheatsSubsystem::UnregisterProvider(UObject* Provider)
{
#if !UE_BUILD_SHIPPING
    if (Provider == nullptr)
    {
        return;
    }

    RegisteredProviders.Remove(Provider);
    UnregisterCheatsByOwner(Provider);
#endif
}

void UNsCheatsSubsystem::RegisterCheatEntry(const FNsCheatEntry& Entry)
{
#if !UE_BUILD_SHIPPING
    if (Entry.Id.IsNone())
    {
        UE_LOG(LogNsCheats, Warning, TEXT("Ignoring cheat entry with empty Id."));
        return;
    }

    Entries.RemoveAll([&Entry](const FNsCheatEntry& Candidate) { return Candidate.Id == Entry.Id; });
    Entries.Add(Entry);
    Entries.Sort([](const FNsCheatEntry& A, const FNsCheatEntry& B)
    {
        if (A.Tab != B.Tab)
        {
            return A.Tab.LexicalLess(B.Tab);
        }
        if (A.Category != B.Category)
        {
            return A.Category.LexicalLess(B.Category);
        }
        return A.SortOrder < B.SortOrder;
    });
    RefreshCheatUI();
#endif
}

void UNsCheatsSubsystem::RegisterButtonCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicExecuteDelegate ExecuteDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.DynamicExecuteDelegate = ExecuteDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::RegisterToggleCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicGetBoolDelegate GetDelegate, FNsCheatDynamicSetBoolDelegate SetDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = ENsCheatEntryType::Toggle;
    Entry.DynamicGetBoolDelegate = GetDelegate;
    Entry.DynamicSetBoolDelegate = SetDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::RegisterDebugTextCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicDebugTextDelegate DebugTextDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = ENsCheatEntryType::DebugView;
    Entry.DynamicDebugTextDelegate = DebugTextDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::RegisterFloatCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicGetFloatDelegate GetDelegate, FNsCheatDynamicSetFloatDelegate SetDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = ENsCheatEntryType::Modifier;
    Entry.DynamicGetFloatDelegate = GetDelegate;
    Entry.DynamicSetFloatDelegate = SetDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::RegisterTextCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicGetStringDelegate GetDelegate, FNsCheatDynamicSetStringDelegate SetDelegate, FNsCheatDynamicExecuteDelegate ExecuteDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = ENsCheatEntryType::Text;
    Entry.DynamicGetStringDelegate = GetDelegate;
    Entry.DynamicSetStringDelegate = SetDelegate;
    Entry.DynamicExecuteDelegate = ExecuteDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::RegisterPickerCheat(const FName Id, const FName Tab, const FName Category, const FText DisplayName, const FText Description, UObject* Owner, FNsCheatDynamicGetPickerOptionsDelegate GetOptionsDelegate, FNsCheatDynamicGetPickerValueDelegate GetValueDelegate, FNsCheatDynamicSetPickerValueDelegate SetValueDelegate, FNsCheatDynamicExecuteDelegate RefreshDelegate, const int32 SortOrder)
{
    FNsCheatEntry Entry;
    Entry.Id = Id;
    Entry.Tab = Tab;
    Entry.Category = Category;
    Entry.DisplayName = DisplayName;
    Entry.Description = Description;
    Entry.Owner = Owner;
    Entry.SortOrder = SortOrder;
    Entry.EntryType = ENsCheatEntryType::Picker;
    Entry.DynamicGetPickerOptionsDelegate = GetOptionsDelegate;
    Entry.DynamicGetPickerValueDelegate = GetValueDelegate;
    Entry.DynamicSetPickerValueDelegate = SetValueDelegate;
    Entry.DynamicExecuteDelegate = RefreshDelegate;
    RegisterCheatEntry(Entry);
}

void UNsCheatsSubsystem::UnregisterCheatsByOwner(UObject* Owner)
{
#if !UE_BUILD_SHIPPING
    Entries.RemoveAll([Owner](const FNsCheatEntry& Entry) { return Entry.Owner.Get() == Owner; });
    RefreshCheatUI();
#endif
}

void UNsCheatsSubsystem::UnregisterCheatEntry(const FName Id)
{
#if !UE_BUILD_SHIPPING
    Entries.RemoveAll([Id](const FNsCheatEntry& Entry) { return Entry.Id == Id; });
    RefreshCheatUI();
#endif
}

bool UNsCheatsSubsystem::ExecuteEntry(const FName Id)
{
#if !UE_BUILD_SHIPPING
    CleanInvalidEntries();
    if (FNsCheatEntry* const Entry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
    {
        return ExecuteEntryAction(*Entry);
    }
#endif
    return false;
}

void UNsCheatsSubsystem::CleanInvalidEntries()
{
#if !UE_BUILD_SHIPPING
    Entries.RemoveAll([](const FNsCheatEntry& Entry) { return Entry.Owner.IsStale(); });
    RegisteredProviders.RemoveAll([](const TWeakObjectPtr<UObject>& Provider) { return Provider.IsStale(); });
#endif
}

void UNsCheatsSubsystem::OpenCheatUI()
{
#if !UE_BUILD_SHIPPING
    if (CheatWindow.IsValid())
    {
        CheatWindow.Pin()->BringToFront();
        return;
    }

    TSharedRef<SWindow> Window = SNew(SWindow)
        .Title(FText::FromString("NsCheats"))
        .ClientSize(FVector2D(760.0f, 680.0f))
        [
            SAssignNew(CheatContentBox, SBox)
            [
                BuildCheatUIContent()
            ]
        ];

    CheatWindow = Window;
    FSlateApplication::Get().AddWindow(Window);
#endif
}

void UNsCheatsSubsystem::CloseCheatUI()
{
#if !UE_BUILD_SHIPPING
    if (const TSharedPtr<SWindow> Window = CheatWindow.Pin())
    {
        Window->RequestDestroyWindow();
    }
    CheatWindow.Reset();
    CheatContentBox.Reset();
#endif
}

void UNsCheatsSubsystem::ToggleCheatUI()
{
#if !UE_BUILD_SHIPPING
    if (CheatWindow.IsValid())
    {
        CloseCheatUI();
    }
    else
    {
        OpenCheatUI();
    }
#endif
}

void UNsCheatsSubsystem::RefreshCheatUI()
{
#if !UE_BUILD_SHIPPING
    if (CheatContentBox.IsValid())
    {
        CheatContentBox->SetContent(BuildCheatUIContent());
    }
#endif
}

FString UNsCheatsSubsystem::DumpRegisteredCheats()
{
    FString Result = TEXT("NsCheats Registered Entries\n");
#if !UE_BUILD_SHIPPING
    CleanInvalidEntries();
    for (const FNsCheatEntry& Entry : Entries)
    {
        Result += FString::Printf(TEXT("- [%s/%s] %s (%s)\n"), *Entry.Tab.ToString(), *Entry.Category.ToString(), *Entry.DisplayName.ToString(), *Entry.Id.ToString());
    }
#endif
    UE_LOG(LogNsCheats, Log, TEXT("%s"), *Result);
    return Result;
}

void UNsCheatsSubsystem::RegisterBuiltInCheats()
{
#if !UE_BUILD_SHIPPING
    ForProvider(this, "Debug")
        .Section("Registry")
        .Button("NsCheats.Dump", FText::FromString("Dump registered cheats"), FNsCheatExecuteDelegate::CreateLambda([this]()
        {
            DumpRegisteredCheats();
        }), FText::FromString("Logs all registered cheat entries."));

    ForProvider(this, "Debug")
        .Section("Player")
        .Button("NsCheats.PlayerLocation", FText::FromString("Log player location"), FNsCheatExecuteDelegate::CreateLambda([this]()
        {
            const UWorld* const World = GetWorld();
            const APlayerController* const PC = World != nullptr ? World->GetFirstPlayerController() : nullptr;
            const APawn* const Pawn = PC != nullptr ? PC->GetPawn() : nullptr;
            if (Pawn != nullptr)
            {
                UE_LOG(LogNsCheats, Log, TEXT("Player location=%s rotation=%s"), *Pawn->GetActorLocation().ToCompactString(), *Pawn->GetActorRotation().ToCompactString());
            }
        }), FText::FromString("Logs the first local player's pawn location and rotation."));

    ForProvider(this, "Performance")
        .Section("Stats")
        .Button("NsCheats.StatFps", FText::FromString("Toggle stat fps"), FNsCheatExecuteDelegate::CreateLambda([this]()
        {
            if (GEngine != nullptr && GetWorld() != nullptr)
            {
                GEngine->Exec(GetWorld(), TEXT("stat fps"));
            }
        }))
        .Button("NsCheats.StatUnit", FText::FromString("Toggle stat unit"), FNsCheatExecuteDelegate::CreateLambda([this]()
        {
            if (GEngine != nullptr && GetWorld() != nullptr)
            {
                GEngine->Exec(GetWorld(), TEXT("stat unit"));
            }
        }));
#endif
}

void UNsCheatsSubsystem::AutoRegisterSubsystemProviders()
{
#if !UE_BUILD_SHIPPING
    UGameInstance* const GameInstance = GetGameInstance();
    if (GameInstance == nullptr)
    {
        return;
    }

    // Subsystems are the only providers auto-discovered. Actors, components, and transient Blueprint objects opt in with RegisterProvider.
    GameInstance->ForEachSubsystem<UGameInstanceSubsystem>([this](UGameInstanceSubsystem* Subsystem)
    {
        if (Subsystem != this)
        {
            RegisterProvider(Subsystem);
        }
    });

    if (UWorld* const World = GetWorld())
    {
        World->ForEachSubsystem<UWorldSubsystem>([this](UWorldSubsystem* Subsystem)
        {
            RegisterProvider(Subsystem);
        });
    }
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildCheatUIContent()
{
#if !UE_BUILD_SHIPPING
    CleanInvalidEntries();
    NormalizeSelection();

    const TArray<FName> Tabs = GetVisibleTabs();
    const TArray<FName> Sections = GetVisibleSectionsForTab(SelectedTab);

    TSharedRef<SVerticalBox> Root = SNew(SVerticalBox);
    Root->AddSlot().AutoHeight().Padding(12.0f, 10.0f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(0.0f, 0.0f, 10.0f, 0.0f)
        [
            SNew(SSearchBox)
            .InitialText(FText::FromString(SearchFilter))
            .HintText(FText::FromString("Search cheats..."))
            .OnTextChanged_Lambda([this](const FText& NewText)
            {
                SearchFilter = NewText.ToString();
                RefreshCheatUI();
            })
        ]
        + SHorizontalBox::Slot().AutoWidth()
        [
            SNew(SButton)
            .Text(FText::FromString("Reload"))
            .OnClicked_Lambda([this]()
            {
                RefreshCheatUI();
                return FReply::Handled();
            })
        ]
    ];

    Root->AddSlot().AutoHeight()
    [
        SNew(SSeparator)
    ];

    Root->AddSlot().AutoHeight().Padding(12.0f, 8.0f, 12.0f, 4.0f)
    [
        BuildTabBar(Tabs)
    ];

    Root->AddSlot().FillHeight(1.0f).Padding(12.0f, 4.0f, 12.0f, 12.0f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 12.0f, 0.0f)
        [
            SNew(SBox)
            .WidthOverride(190.0f)
            [
                BuildSectionList(Sections)
            ]
        ]
        + SHorizontalBox::Slot().FillWidth(1.0f)
        [
            BuildEntriesPanel()
        ]
    ];

    return Root;
#else
    return SNew(STextBlock).Text(FText::FromString("NsCheats is unavailable in Shipping."));
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildTabBar(const TArray<FName>& Tabs)
{
#if !UE_BUILD_SHIPPING
    TSharedRef<SHorizontalBox> TabBar = SNew(SHorizontalBox);
    for (const FName Tab : Tabs)
    {
        const bool bSelected = Tab == SelectedTab;
        TabBar->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
        [
            SNew(SButton)
            .ButtonColorAndOpacity(bSelected ? FLinearColor(0.16f, 0.30f, 0.52f, 1.0f) : FLinearColor(0.06f, 0.07f, 0.08f, 1.0f))
            .Text(FText::FromName(Tab))
            .OnClicked_Lambda([this, Tab]()
            {
                SelectedTab = Tab;
                const TArray<FName> Sections = GetVisibleSectionsForTab(SelectedTab);
                SelectedCategory = Sections.Num() > 0 ? Sections[0] : NAME_None;
                RefreshCheatUI();
                return FReply::Handled();
            })
        ];
    }

    if (Tabs.Num() == 0)
    {
        TabBar->AddSlot().AutoWidth()
        [
            SNew(STextBlock)
            .Text(FText::FromString("No cheats registered."))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.72f, 0.72f)))
        ];
    }

    return TabBar;
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildSectionList(const TArray<FName>& Sections)
{
#if !UE_BUILD_SHIPPING
    TSharedRef<SVerticalBox> List = SNew(SVerticalBox);
    List->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
    [
        SNew(STextBlock)
        .Text(FText::FromString("Sections"))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
        .ColorAndOpacity(FSlateColor(FLinearColor(0.68f, 0.74f, 0.82f)))
    ];

    for (const FName Section : Sections)
    {
        const bool bSelected = Section == SelectedCategory;
        List->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
        [
            SNew(SButton)
            .HAlign(HAlign_Left)
            .ButtonColorAndOpacity(bSelected ? FLinearColor(0.16f, 0.22f, 0.31f, 1.0f) : FLinearColor(0.045f, 0.05f, 0.06f, 1.0f))
            .Text(FText::FromName(Section))
            .OnClicked_Lambda([this, Section]()
            {
                SelectedCategory = Section;
                RefreshCheatUI();
                return FReply::Handled();
            })
        ];
    }

    if (Sections.Num() == 0)
    {
        List->AddSlot().AutoHeight()
        [
            SNew(STextBlock)
            .Text(FText::FromString("No sections."))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.64f, 0.64f, 0.64f)))
        ];
    }

    return SNew(SBorder)
        .Padding(8.0f)
        .BorderBackgroundColor(FLinearColor(0.025f, 0.028f, 0.032f, 1.0f))
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                List
            ]
        ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildEntriesPanel()
{
#if !UE_BUILD_SHIPPING
    const TArray<FNsCheatEntry> VisibleEntries = GetVisibleEntriesForSelection();
    TSharedRef<SVerticalBox> EntryList = SNew(SVerticalBox);

    EntryList->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(SelectedCategory.IsNone()
                ? FText::FromName(SelectedTab)
                : FText::Format(FText::FromString("{0} / {1}"), FText::FromName(SelectedTab), FText::FromName(SelectedCategory)))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
        [
            SNew(STextBlock)
            .Text(FText::Format(FText::FromString("{0} entries"), VisibleEntries.Num()))
            .ColorAndOpacity(FSlateColor(FLinearColor(0.62f, 0.68f, 0.74f)))
        ]
    ];

    if (VisibleEntries.Num() == 0)
    {
        EntryList->AddSlot().AutoHeight()
        [
            SNew(SBorder)
            .Padding(14.0f)
            .BorderBackgroundColor(FLinearColor(0.03f, 0.033f, 0.038f, 1.0f))
            [
                SNew(STextBlock)
                .Text(FText::FromString("No cheats match this tab, section, and search."))
                .ColorAndOpacity(FSlateColor(FLinearColor(0.72f, 0.72f, 0.72f)))
            ]
        ];
    }
    else
    {
        for (const FNsCheatEntry& Entry : VisibleEntries)
        {
            EntryList->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
            [
                BuildCheatEntryWidget(Entry)
            ];
        }
    }

    return SNew(SBorder)
        .Padding(10.0f)
        .BorderBackgroundColor(FLinearColor(0.018f, 0.020f, 0.024f, 1.0f))
        [
            SNew(SScrollBox)
            + SScrollBox::Slot()
            [
                EntryList
            ]
        ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildCheatEntryWidget(const FNsCheatEntry& Entry)
{
#if !UE_BUILD_SHIPPING
    if (Entry.EntryType == ENsCheatEntryType::Picker)
    {
        return BuildPickerWidget(Entry);
    }
    if (Entry.EntryType == ENsCheatEntryType::Text)
    {
        return BuildTextWidget(Entry);
    }
    if (Entry.EntryType == ENsCheatEntryType::ButtonRow)
    {
        return BuildButtonRowWidget(Entry);
    }

    const bool bHasAction = Entry.ExecuteDelegate.IsBound() || Entry.DynamicExecuteDelegate.IsBound();
    const bool bHasDebugText = Entry.DebugTextDelegate.IsBound() || Entry.DynamicDebugTextDelegate.IsBound();
    const bool bHasFloatModifier = Entry.GetFloatDelegate.IsBound() || Entry.DynamicGetFloatDelegate.IsBound();
    const bool bHasToggle = Entry.EntryType == ENsCheatEntryType::Toggle;

    return SNew(SBorder)
    .Padding(FMargin(10.0f, 9.0f))
    .BorderBackgroundColor(FLinearColor(0.035f, 0.039f, 0.046f, 1.0f))
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Text(Entry.DisplayName)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
        [
            SNew(STextBlock)
            .Text(Entry.Description.IsEmpty() ? FText::FromString(Entry.Id.ToString()) : Entry.Description)
            .AutoWrapText(true)
            .ColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.73f, 0.78f)))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [
                SNew(STextBlock)
                .Visibility(bHasDebugText ? EVisibility::Visible : EVisibility::Collapsed)
                .Text(GetEntryDebugText(Entry))
                .AutoWrapText(true)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.80f, 0.86f, 0.92f)))
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f)
            [
                SNew(SBox)
                .WidthOverride(160.0f)
                .Visibility(bHasFloatModifier ? EVisibility::Visible : EVisibility::Collapsed)
                [
                    SNew(SNumericEntryBox<float>)
                    .Value_Lambda([this, Id = Entry.Id]() -> TOptional<float>
                    {
                        const FNsCheatEntry* const FoundEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; });
                        return FoundEntry != nullptr ? GetEntryFloatValue(*FoundEntry) : TOptional<float>();
                    })
                    .OnValueCommitted_Lambda([this, Id = Entry.Id](const float NewValue, ETextCommit::Type)
                    {
                        if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                        {
                            SetEntryFloatValue(*MutableEntry, NewValue);
                            RefreshCheatUI();
                        }
                    })
                ]
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f)
            [
                SNew(SCheckBox)
                .Visibility(bHasToggle ? EVisibility::Visible : EVisibility::Collapsed)
                .IsChecked_Lambda([this, Id = Entry.Id]()
                {
                    const FNsCheatEntry* const FoundEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; });
                    return FoundEntry != nullptr && GetEntryBoolValue(*FoundEntry) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
                })
                .OnCheckStateChanged_Lambda([this, Id = Entry.Id](const ECheckBoxState NewState)
                {
                    if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                    {
                        SetEntryBoolValue(*MutableEntry, NewState == ECheckBoxState::Checked);
                        RefreshCheatUI();
                    }
                })
            ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [
                SNew(SButton)
                .Visibility(bHasAction && !bHasToggle ? EVisibility::Visible : EVisibility::Collapsed)
                .Text(FText::FromString("Run"))
                .ToolTipText(Entry.Description)
                .OnClicked_Lambda([this, Id = Entry.Id]()
                {
                    if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                    {
                        ExecuteEntryAction(*MutableEntry);
                    }
                    RefreshCheatUI();
                    return FReply::Handled();
                })
            ]
        ]
    ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildPickerWidget(const FNsCheatEntry& Entry)
{
#if !UE_BUILD_SHIPPING
    TArray<TSharedPtr<FNsCheatPickerOption>>& CachedOptions = PickerOptionsByEntryId.FindOrAdd(Entry.Id);
    CachedOptions.Reset();
    for (const FNsCheatPickerOption& Option : GetEntryPickerOptions(Entry))
    {
        CachedOptions.Add(MakeShared<FNsCheatPickerOption>(Option));
    }

    const FName CurrentValue = GetEntryPickerValue(Entry);
    TSharedPtr<FNsCheatPickerOption> SelectedOption;
    for (const TSharedPtr<FNsCheatPickerOption>& Option : CachedOptions)
    {
        if (Option.IsValid() && Option->Value == CurrentValue)
        {
            SelectedOption = Option;
            break;
        }
    }
    if (!SelectedOption.IsValid() && CachedOptions.Num() > 0)
    {
        SelectedOption = CachedOptions[0];
    }

    return SNew(SBorder)
        .Padding(FMargin(10.0f, 9.0f))
        .BorderBackgroundColor(FLinearColor(0.035f, 0.039f, 0.046f, 1.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(STextBlock)
                .Text(Entry.DisplayName)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(Entry.Description.IsEmpty() ? FText::FromString(Entry.Id.ToString()) : Entry.Description)
                .AutoWrapText(true)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.73f, 0.78f)))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [
                    SNew(SComboBox<TSharedPtr<FNsCheatPickerOption>>)
                    .OptionsSource(&CachedOptions)
                    .InitiallySelectedItem(SelectedOption)
                    .OnGenerateWidget_Lambda([](TSharedPtr<FNsCheatPickerOption> Option)
                    {
                        return SNew(STextBlock).Text(Option.IsValid() ? Option->Label : FText::GetEmpty());
                    })
                    .OnSelectionChanged_Lambda([this, Id = Entry.Id](TSharedPtr<FNsCheatPickerOption> Option, ESelectInfo::Type)
                    {
                        if (Option.IsValid())
                        {
                            if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                            {
                                SetEntryPickerValue(*MutableEntry, Option->Value);
                                RefreshCheatUI();
                            }
                        }
                    })
                    [
                        SNew(STextBlock)
                        .Text_Lambda([SelectedOption]()
                        {
                            return SelectedOption.IsValid() ? SelectedOption->Label : FText::FromString("None");
                        })
                    ]
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Visibility((Entry.ExecuteDelegate.IsBound() || Entry.DynamicExecuteDelegate.IsBound()) ? EVisibility::Visible : EVisibility::Collapsed)
                    .Text(FText::FromString("Run"))
                    .OnClicked_Lambda([this, Id = Entry.Id]()
                    {
                        if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                        {
                            ExecuteEntryAction(*MutableEntry);
                            RefreshCheatUI();
                        }
                        return FReply::Handled();
                    })
                ]
            ]
        ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildTextWidget(const FNsCheatEntry& Entry)
{
#if !UE_BUILD_SHIPPING
    return SNew(SBorder)
        .Padding(FMargin(10.0f, 9.0f))
        .BorderBackgroundColor(FLinearColor(0.035f, 0.039f, 0.046f, 1.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(STextBlock)
                .Text(Entry.DisplayName)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(Entry.Description.IsEmpty() ? FText::FromString(Entry.Id.ToString()) : Entry.Description)
                .AutoWrapText(true)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.73f, 0.78f)))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [
                    SNew(SEditableTextBox)
                    .Text_Lambda([this, Id = Entry.Id]()
                    {
                        const FNsCheatEntry* const FoundEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; });
                        return FText::FromString(FoundEntry != nullptr ? GetEntryStringValue(*FoundEntry) : FString());
                    })
                    .OnTextCommitted_Lambda([this, Id = Entry.Id](const FText& NewText, ETextCommit::Type)
                    {
                        if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                        {
                            SetEntryStringValue(*MutableEntry, NewText.ToString());
                        }
                    })
                ]
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SButton)
                    .Visibility((Entry.ExecuteDelegate.IsBound() || Entry.DynamicExecuteDelegate.IsBound()) ? EVisibility::Visible : EVisibility::Collapsed)
                    .Text(FText::FromString("Run"))
                    .ToolTipText(Entry.Description)
                    .OnClicked_Lambda([this, Id = Entry.Id]()
                    {
                        if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                        {
                            ExecuteEntryAction(*MutableEntry);
                        }
                        RefreshCheatUI();
                        return FReply::Handled();
                    })
                ]
            ]
        ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TSharedRef<SWidget> UNsCheatsSubsystem::BuildButtonRowWidget(const FNsCheatEntry& Entry)
{
#if !UE_BUILD_SHIPPING
    TSharedRef<SHorizontalBox> ButtonRow = SNew(SHorizontalBox);
    for (const FNsCheatInlineAction& Action : Entry.InlineActions)
    {
        ButtonRow->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
        [
            SNew(SButton)
            .Text(Action.Label)
            .ToolTipText(Action.Description)
            .OnClicked_Lambda([this, Id = Entry.Id, ActionId = Action.Id]()
            {
                if (FNsCheatEntry* const MutableEntry = Entries.FindByPredicate([Id](const FNsCheatEntry& Candidate) { return Candidate.Id == Id; }))
                {
                    if (FNsCheatInlineAction* const MutableAction = MutableEntry->InlineActions.FindByPredicate([ActionId](const FNsCheatInlineAction& Candidate) { return Candidate.Id == ActionId; }))
                    {
                        if (MutableAction->ExecuteDelegate.IsBound())
                        {
                            MutableAction->ExecuteDelegate.Execute();
                        }
                    }
                }
                RefreshCheatUI();
                return FReply::Handled();
            })
        ];
    }

    return SNew(SBorder)
        .Padding(FMargin(10.0f, 9.0f))
        .BorderBackgroundColor(FLinearColor(0.035f, 0.039f, 0.046f, 1.0f))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(STextBlock)
                .Text(Entry.DisplayName)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(Entry.Description.IsEmpty() ? FText::FromString(Entry.Id.ToString()) : Entry.Description)
                .AutoWrapText(true)
                .ColorAndOpacity(FSlateColor(FLinearColor(0.70f, 0.73f, 0.78f)))
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [
                ButtonRow
            ]
        ];
#else
    return SNew(STextBlock).Text(FText::GetEmpty());
#endif
}

TArray<FName> UNsCheatsSubsystem::GetVisibleTabs() const
{
    TArray<FName> Tabs;
#if !UE_BUILD_SHIPPING
    for (const FNsCheatEntry& Entry : Entries)
    {
        if (EntryMatchesSearch(Entry))
        {
            Tabs.AddUnique(Entry.Tab);
        }
    }
    Tabs.Sort([](const FName A, const FName B) { return A.LexicalLess(B); });
#endif
    return Tabs;
}

TArray<FName> UNsCheatsSubsystem::GetVisibleSectionsForTab(const FName Tab) const
{
    TArray<FName> Sections;
#if !UE_BUILD_SHIPPING
    for (const FNsCheatEntry& Entry : Entries)
    {
        if (Entry.Tab == Tab && EntryMatchesSearch(Entry))
        {
            Sections.AddUnique(Entry.Category);
        }
    }
    Sections.Sort([](const FName A, const FName B) { return A.LexicalLess(B); });
#endif
    return Sections;
}

TArray<FNsCheatEntry> UNsCheatsSubsystem::GetVisibleEntriesForSelection() const
{
    TArray<FNsCheatEntry> Results;
#if !UE_BUILD_SHIPPING
    for (const FNsCheatEntry& Entry : Entries)
    {
        if (Entry.Tab == SelectedTab && Entry.Category == SelectedCategory && EntryMatchesSearch(Entry))
        {
            Results.Add(Entry);
        }
    }
    Results.Sort([](const FNsCheatEntry& A, const FNsCheatEntry& B)
    {
        if (A.SortOrder != B.SortOrder)
        {
            return A.SortOrder < B.SortOrder;
        }
        return A.DisplayName.ToString() < B.DisplayName.ToString();
    });
#endif
    return Results;
}

bool UNsCheatsSubsystem::EntryMatchesSearch(const FNsCheatEntry& Entry) const
{
#if !UE_BUILD_SHIPPING
    if (SearchFilter.IsEmpty())
    {
        return true;
    }
    const FString SearchBlob = FString::Printf(TEXT("%s %s %s %s %s"),
        *Entry.DisplayName.ToString(),
        *Entry.Description.ToString(),
        *Entry.Id.ToString(),
        *Entry.Tab.ToString(),
        *Entry.Category.ToString());
    return SearchBlob.Contains(SearchFilter, ESearchCase::IgnoreCase);
#else
    return false;
#endif
}

void UNsCheatsSubsystem::NormalizeSelection()
{
#if !UE_BUILD_SHIPPING
    const TArray<FName> Tabs = GetVisibleTabs();
    if (Tabs.Num() == 0)
    {
        SelectedTab = NAME_None;
        SelectedCategory = NAME_None;
        return;
    }

    if (!Tabs.Contains(SelectedTab))
    {
        SelectedTab = Tabs[0];
    }

    const TArray<FName> Sections = GetVisibleSectionsForTab(SelectedTab);
    if (Sections.Num() == 0)
    {
        SelectedCategory = NAME_None;
        return;
    }

    if (!Sections.Contains(SelectedCategory))
    {
        SelectedCategory = Sections[0];
    }
#endif
}

bool UNsCheatsSubsystem::ExecuteEntryAction(FNsCheatEntry& Entry)
{
    if (Entry.ExecuteDelegate.IsBound())
    {
        Entry.ExecuteDelegate.Execute();
        return true;
    }
    if (Entry.DynamicExecuteDelegate.IsBound())
    {
        Entry.DynamicExecuteDelegate.Execute();
        return true;
    }
    return false;
}

FText UNsCheatsSubsystem::GetEntryDebugText(const FNsCheatEntry& Entry) const
{
    if (Entry.DebugTextDelegate.IsBound())
    {
        return Entry.DebugTextDelegate.Execute();
    }
    if (Entry.DynamicDebugTextDelegate.IsBound())
    {
        return Entry.DynamicDebugTextDelegate.Execute();
    }
    return FText::GetEmpty();
}

TOptional<float> UNsCheatsSubsystem::GetEntryFloatValue(const FNsCheatEntry& Entry) const
{
    if (Entry.GetFloatDelegate.IsBound())
    {
        return Entry.GetFloatDelegate.Execute();
    }
    if (Entry.DynamicGetFloatDelegate.IsBound())
    {
        return Entry.DynamicGetFloatDelegate.Execute();
    }
    return TOptional<float>();
}

void UNsCheatsSubsystem::SetEntryFloatValue(FNsCheatEntry& Entry, const float Value)
{
    if (Entry.SetFloatDelegate.IsBound())
    {
        Entry.SetFloatDelegate.Execute(Value);
    }
    else if (Entry.DynamicSetFloatDelegate.IsBound())
    {
        Entry.DynamicSetFloatDelegate.Execute(Value);
    }
}

bool UNsCheatsSubsystem::GetEntryBoolValue(const FNsCheatEntry& Entry) const
{
    if (Entry.GetBoolDelegate.IsBound())
    {
        return Entry.GetBoolDelegate.Execute();
    }
    if (Entry.DynamicGetBoolDelegate.IsBound())
    {
        return Entry.DynamicGetBoolDelegate.Execute();
    }
    return false;
}

void UNsCheatsSubsystem::SetEntryBoolValue(FNsCheatEntry& Entry, const bool bValue)
{
    if (Entry.SetBoolDelegate.IsBound())
    {
        Entry.SetBoolDelegate.Execute(bValue);
    }
    else if (Entry.DynamicSetBoolDelegate.IsBound())
    {
        Entry.DynamicSetBoolDelegate.Execute(bValue);
    }
}

FString UNsCheatsSubsystem::GetEntryStringValue(const FNsCheatEntry& Entry) const
{
    if (Entry.GetStringDelegate.IsBound())
    {
        return Entry.GetStringDelegate.Execute();
    }
    if (Entry.DynamicGetStringDelegate.IsBound())
    {
        return Entry.DynamicGetStringDelegate.Execute();
    }
    return FString();
}

void UNsCheatsSubsystem::SetEntryStringValue(FNsCheatEntry& Entry, const FString& Value)
{
    if (Entry.SetStringDelegate.IsBound())
    {
        Entry.SetStringDelegate.Execute(Value);
    }
    else if (Entry.DynamicSetStringDelegate.IsBound())
    {
        Entry.DynamicSetStringDelegate.Execute(Value);
    }
}

TArray<FNsCheatPickerOption> UNsCheatsSubsystem::GetEntryPickerOptions(const FNsCheatEntry& Entry) const
{
    if (Entry.GetPickerOptionsDelegate.IsBound())
    {
        return Entry.GetPickerOptionsDelegate.Execute();
    }
    if (Entry.DynamicGetPickerOptionsDelegate.IsBound())
    {
        return Entry.DynamicGetPickerOptionsDelegate.Execute();
    }
    return TArray<FNsCheatPickerOption>();
}

FName UNsCheatsSubsystem::GetEntryPickerValue(const FNsCheatEntry& Entry) const
{
    if (Entry.GetPickerValueDelegate.IsBound())
    {
        return Entry.GetPickerValueDelegate.Execute();
    }
    if (Entry.DynamicGetPickerValueDelegate.IsBound())
    {
        return Entry.DynamicGetPickerValueDelegate.Execute();
    }
    return NAME_None;
}

void UNsCheatsSubsystem::SetEntryPickerValue(FNsCheatEntry& Entry, const FName Value)
{
    if (Entry.SetPickerValueDelegate.IsBound())
    {
        Entry.SetPickerValueDelegate.Execute(Value);
    }
    else if (Entry.DynamicSetPickerValueDelegate.IsBound())
    {
        Entry.DynamicSetPickerValueDelegate.Execute(Value);
    }
}

void UNsCheatsSubsystem::RegisterConsoleCommands()
{
#if !UE_BUILD_SHIPPING
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    const auto RegisterIfAbsent = [this, &ConsoleManager](const TCHAR* Name, TFunction<IConsoleObject*()> Register)
    {
        // Console commands are global, but PIE can create more than one game instance.
        // Only the subsystem that creates a command owns its handle and can unregister it.
        if (ConsoleManager.FindConsoleObject(Name) == nullptr)
        {
            if (IConsoleObject* ConsoleObject = Register())
            {
                RegisteredConsoleCommands.Add(ConsoleObject);
            }
        }
    };

    RegisterIfAbsent(TEXT("ns.Cheats.Open"), [&ConsoleManager, this]() -> IConsoleObject*
    {
        return ConsoleManager.RegisterConsoleCommand(TEXT("ns.Cheats.Open"), TEXT("Open NsCheats."), FConsoleCommandDelegate::CreateUObject(this, &UNsCheatsSubsystem::OpenCheatUI));
    });
    RegisterIfAbsent(TEXT("ns.Cheats.Close"), [&ConsoleManager, this]() -> IConsoleObject*
    {
        return ConsoleManager.RegisterConsoleCommand(TEXT("ns.Cheats.Close"), TEXT("Close NsCheats."), FConsoleCommandDelegate::CreateUObject(this, &UNsCheatsSubsystem::CloseCheatUI));
    });
    RegisterIfAbsent(TEXT("ns.Cheats.Toggle"), [&ConsoleManager, this]() -> IConsoleObject*
    {
        return ConsoleManager.RegisterConsoleCommand(TEXT("ns.Cheats.Toggle"), TEXT("Toggle NsCheats."), FConsoleCommandDelegate::CreateUObject(this, &UNsCheatsSubsystem::ToggleCheatUI));
    });
    RegisterIfAbsent(TEXT("ns.Cheats.Dump"), [&ConsoleManager, this]() -> IConsoleObject*
    {
        return ConsoleManager.RegisterConsoleCommand(TEXT("ns.Cheats.Dump"), TEXT("Dump NsCheats entries."), FConsoleCommandDelegate::CreateLambda([this]()
        {
            DumpRegisteredCheats();
        }));
    });
    RegisterIfAbsent(TEXT("ns.Cheats.Exec"), [&ConsoleManager, this]() -> IConsoleObject*
    {
        return ConsoleManager.RegisterConsoleCommand(TEXT("ns.Cheats.Exec"), TEXT("Execute a console command. Usage: ns.Cheats.Exec stat fps"), FConsoleCommandWithArgsDelegate::CreateLambda([this](const TArray<FString>& Args)
        {
            if (Args.Num() == 0 || GEngine == nullptr || GetWorld() == nullptr)
            {
                UE_LOG(LogNsCheats, Warning, TEXT("Usage: ns.Cheats.Exec <Command>"));
                return;
            }

            const FString Command = FString::Join(Args, TEXT(" "));
            GEngine->Exec(GetWorld(), *Command);
        }));
    });
#endif
}

void UNsCheatsSubsystem::UnregisterConsoleCommands()
{
#if !UE_BUILD_SHIPPING
    IConsoleManager& ConsoleManager = IConsoleManager::Get();
    for (IConsoleObject* const ConsoleObject : RegisteredConsoleCommands)
    {
        if (ConsoleObject != nullptr)
        {
            ConsoleManager.UnregisterConsoleObject(ConsoleObject);
        }
    }
    RegisteredConsoleCommands.Reset();
#endif
}
