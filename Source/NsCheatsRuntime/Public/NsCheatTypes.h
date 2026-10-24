#pragma once

#include "CoreMinimal.h"
#include "NsCheatTypes.generated.h"

class UNsCheatsSubsystem;

USTRUCT(BlueprintType)
struct NSCHEATSRUNTIME_API FNsCheatPickerOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FName Value;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FText Label;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FText Description;
};

DECLARE_DELEGATE_RetVal(FText, FNsCheatDebugTextDelegate);
DECLARE_DELEGATE(FNsCheatExecuteDelegate);
DECLARE_DELEGATE_RetVal(float, FNsCheatGetFloatDelegate);
DECLARE_DELEGATE_OneParam(FNsCheatSetFloatDelegate, float);
DECLARE_DELEGATE_RetVal(bool, FNsCheatGetBoolDelegate);
DECLARE_DELEGATE_OneParam(FNsCheatSetBoolDelegate, bool);
DECLARE_DELEGATE_RetVal(FString, FNsCheatGetStringDelegate);
DECLARE_DELEGATE_OneParam(FNsCheatSetStringDelegate, const FString&);
DECLARE_DELEGATE_RetVal(TArray<FNsCheatPickerOption>, FNsCheatGetPickerOptionsDelegate);
DECLARE_DELEGATE_RetVal(FName, FNsCheatGetPickerValueDelegate);
DECLARE_DELEGATE_OneParam(FNsCheatSetPickerValueDelegate, FName);

DECLARE_DYNAMIC_DELEGATE(FNsCheatDynamicExecuteDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FNsCheatDynamicDebugTextDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal(float, FNsCheatDynamicGetFloatDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FNsCheatDynamicSetFloatDelegate, float, Value);
DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FNsCheatDynamicGetBoolDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FNsCheatDynamicSetBoolDelegate, bool, bValue);
DECLARE_DYNAMIC_DELEGATE_RetVal(FString, FNsCheatDynamicGetStringDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FNsCheatDynamicSetStringDelegate, const FString&, Value);
DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FNsCheatPickerOption>, FNsCheatDynamicGetPickerOptionsDelegate);
DECLARE_DYNAMIC_DELEGATE_RetVal(FName, FNsCheatDynamicGetPickerValueDelegate);
DECLARE_DYNAMIC_DELEGATE_OneParam(FNsCheatDynamicSetPickerValueDelegate, FName, Value);

struct NSCHEATSRUNTIME_API FNsCheatInlineAction
{
    FName Id;
    FText Label;
    FText Description;
    FNsCheatExecuteDelegate ExecuteDelegate;
};

UENUM(BlueprintType)
enum class ENsCheatEntryType : uint8
{
    Button,
    Modifier,
    DebugView,
    Toggle,
    Picker,
    Text,
    ButtonRow
};

USTRUCT(BlueprintType)
struct NSCHEATSRUNTIME_API FNsCheatEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FName Id;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FName Tab = "Debug";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    FName Category = "General";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    int32 SortOrder = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="NsCheats")
    ENsCheatEntryType EntryType = ENsCheatEntryType::Button;

    UPROPERTY(BlueprintReadOnly, Category="NsCheats")
    TWeakObjectPtr<UObject> Owner;

    FNsCheatExecuteDelegate ExecuteDelegate;
    FNsCheatDebugTextDelegate DebugTextDelegate;
    FNsCheatGetFloatDelegate GetFloatDelegate;
    FNsCheatSetFloatDelegate SetFloatDelegate;
    FNsCheatGetBoolDelegate GetBoolDelegate;
    FNsCheatSetBoolDelegate SetBoolDelegate;
    FNsCheatGetStringDelegate GetStringDelegate;
    FNsCheatSetStringDelegate SetStringDelegate;
    FNsCheatGetPickerOptionsDelegate GetPickerOptionsDelegate;
    FNsCheatGetPickerValueDelegate GetPickerValueDelegate;
    FNsCheatSetPickerValueDelegate SetPickerValueDelegate;
    TArray<FNsCheatInlineAction> InlineActions;

    FNsCheatDynamicExecuteDelegate DynamicExecuteDelegate;
    FNsCheatDynamicDebugTextDelegate DynamicDebugTextDelegate;
    FNsCheatDynamicGetFloatDelegate DynamicGetFloatDelegate;
    FNsCheatDynamicSetFloatDelegate DynamicSetFloatDelegate;
    FNsCheatDynamicGetBoolDelegate DynamicGetBoolDelegate;
    FNsCheatDynamicSetBoolDelegate DynamicSetBoolDelegate;
    FNsCheatDynamicGetStringDelegate DynamicGetStringDelegate;
    FNsCheatDynamicSetStringDelegate DynamicSetStringDelegate;
    FNsCheatDynamicGetPickerOptionsDelegate DynamicGetPickerOptionsDelegate;
    FNsCheatDynamicGetPickerValueDelegate DynamicGetPickerValueDelegate;
    FNsCheatDynamicSetPickerValueDelegate DynamicSetPickerValueDelegate;
};
