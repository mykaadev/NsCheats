#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NsCheatProviderInterface.generated.h"

class UNsCheatsSubsystem;

UINTERFACE(BlueprintType)
class NSCHEATSRUNTIME_API UNsCheatProviderInterface : public UInterface
{
    GENERATED_BODY()
};

class NSCHEATSRUNTIME_API INsCheatProviderInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="NsCheats")
    void RegisterCheats(UNsCheatsSubsystem* CheatsSubsystem);
};
