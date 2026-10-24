#pragma once

#include "CoreMinimal.h"
#include "NsCheatTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "NsCheatsSubsystem.generated.h"

class SBox;
class SWidget;
class SWindow;
class UNsCheatsSubsystem;

class NSCHEATSRUNTIME_API FNsCheatBuilder
{
public:
    FNsCheatBuilder(UNsCheatsSubsystem* InSubsystem, UObject* InOwner, FName InTab);

    /** Selects the visible section used by following entries. */
    FNsCheatBuilder& Section(FName InSection);

    /** Registers a click action. */
    FNsCheatBuilder& Button(FName Id, FText Label, FNsCheatExecuteDelegate Execute, FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers a boolean action shown as a toggle row. */
    FNsCheatBuilder& Toggle(FName Id, FText Label, FNsCheatGetBoolDelegate GetValue, FNsCheatSetBoolDelegate SetValue, FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers read-only text that refreshes with the cheat UI. */
    FNsCheatBuilder& DebugText(FName Id, FText Label, FNsCheatDebugTextDelegate DebugText, FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers an editable float field. */
    FNsCheatBuilder& Float(FName Id, FText Label, FNsCheatGetFloatDelegate GetValue, FNsCheatSetFloatDelegate SetValue, FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers an editable text field. */
    FNsCheatBuilder& Text(FName Id, FText Label, FNsCheatGetStringDelegate GetValue, FNsCheatSetStringDelegate SetValue, FNsCheatExecuteDelegate Execute = FNsCheatExecuteDelegate(), FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers a reusable dropdown picker. */
    FNsCheatBuilder& Picker(FName Id, FText Label, FNsCheatGetPickerOptionsDelegate GetOptions, FNsCheatGetPickerValueDelegate GetValue, FNsCheatSetPickerValueDelegate SetValue, FNsCheatExecuteDelegate Refresh = FNsCheatExecuteDelegate(), FText Description = FText::GetEmpty(), int32 SortOrder = 0);

    /** Registers a compact row of related buttons. */
    FNsCheatBuilder& ButtonRow(FName Id, FText Label, TArray<FNsCheatInlineAction> Actions, FText Description = FText::GetEmpty(), int32 SortOrder = 0);

private:
    FNsCheatEntry MakeEntry(FName Id, FText Label, FText Description, ENsCheatEntryType Type, int32 SortOrder) const;
    void Register(FNsCheatEntry&& Entry) const;

    UNsCheatsSubsystem* Subsystem = nullptr;
    UObject* Owner = nullptr;
    FName Tab;
    FName CurrentSection = "General";
};

UCLASS()
class NSCHEATSRUNTIME_API UNsCheatsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Starts a fluent native registration block for a provider object. */
    FNsCheatBuilder ForProvider(UObject* Owner, FName Tab);

    /** Registers all cheats exposed by an object implementing INsCheatProviderInterface. */
    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterProvider(UObject* Provider);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void UnregisterProvider(UObject* Provider);

    void RegisterCheatEntry(const FNsCheatEntry& Entry);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterButtonCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicExecuteDelegate ExecuteDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterToggleCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicGetBoolDelegate GetDelegate, FNsCheatDynamicSetBoolDelegate SetDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterDebugTextCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicDebugTextDelegate DebugTextDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterFloatCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicGetFloatDelegate GetDelegate, FNsCheatDynamicSetFloatDelegate SetDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterTextCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicGetStringDelegate GetDelegate, FNsCheatDynamicSetStringDelegate SetDelegate, FNsCheatDynamicExecuteDelegate ExecuteDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RegisterPickerCheat(FName Id, FName Tab, FName Category, FText DisplayName, FText Description, UObject* Owner, FNsCheatDynamicGetPickerOptionsDelegate GetOptionsDelegate, FNsCheatDynamicGetPickerValueDelegate GetValueDelegate, FNsCheatDynamicSetPickerValueDelegate SetValueDelegate, FNsCheatDynamicExecuteDelegate RefreshDelegate, int32 SortOrder = 0);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void UnregisterCheatEntry(FName Id);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void UnregisterCheatsByOwner(UObject* Owner);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    bool ExecuteEntry(FName Id);

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void CleanInvalidEntries();

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void OpenCheatUI();

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void CloseCheatUI();

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void ToggleCheatUI();

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    void RefreshCheatUI();

    UFUNCTION(BlueprintCallable, Category="NsCheats")
    FString DumpRegisteredCheats();

    const TArray<FNsCheatEntry>& GetEntries() const { return Entries; }

private:
    void AutoRegisterSubsystemProviders();
    void RegisterBuiltInCheats();
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();
    TSharedRef<SWidget> BuildCheatUIContent();
    TSharedRef<SWidget> BuildTabBar(const TArray<FName>& Tabs);
    TSharedRef<SWidget> BuildSectionList(const TArray<FName>& Sections);
    TSharedRef<SWidget> BuildEntriesPanel();
    TSharedRef<SWidget> BuildCheatEntryWidget(const FNsCheatEntry& Entry);
    TSharedRef<SWidget> BuildPickerWidget(const FNsCheatEntry& Entry);
    TSharedRef<SWidget> BuildTextWidget(const FNsCheatEntry& Entry);
    TSharedRef<SWidget> BuildButtonRowWidget(const FNsCheatEntry& Entry);
    TArray<FName> GetVisibleTabs() const;
    TArray<FName> GetVisibleSectionsForTab(FName Tab) const;
    TArray<FNsCheatEntry> GetVisibleEntriesForSelection() const;
    bool EntryMatchesSearch(const FNsCheatEntry& Entry) const;
    void NormalizeSelection();
    bool ExecuteEntryAction(FNsCheatEntry& Entry);
    FText GetEntryDebugText(const FNsCheatEntry& Entry) const;
    TOptional<float> GetEntryFloatValue(const FNsCheatEntry& Entry) const;
    void SetEntryFloatValue(FNsCheatEntry& Entry, float Value);
    bool GetEntryBoolValue(const FNsCheatEntry& Entry) const;
    void SetEntryBoolValue(FNsCheatEntry& Entry, bool bValue);
    FString GetEntryStringValue(const FNsCheatEntry& Entry) const;
    void SetEntryStringValue(FNsCheatEntry& Entry, const FString& Value);
    TArray<FNsCheatPickerOption> GetEntryPickerOptions(const FNsCheatEntry& Entry) const;
    FName GetEntryPickerValue(const FNsCheatEntry& Entry) const;
    void SetEntryPickerValue(FNsCheatEntry& Entry, FName Value);

    UPROPERTY()
    TArray<FNsCheatEntry> Entries;

    UPROPERTY()
    TArray<TWeakObjectPtr<UObject>> RegisteredProviders;

    TArray<IConsoleObject*> RegisteredConsoleCommands;
    TWeakPtr<SWindow> CheatWindow;
    TSharedPtr<SBox> CheatContentBox;
    FString SearchFilter;
    FName SelectedTab;
    FName SelectedCategory;
    TMap<FName, TArray<TSharedPtr<FNsCheatPickerOption>>> PickerOptionsByEntryId;
};
