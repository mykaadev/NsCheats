#include "NsCheatWorldSubsystem.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "NsCheatsSubsystem.h"

bool UNsCheatWorldSubsystem::ShouldCreateSubsystem(UObject* const Outer) const
{
#if UE_BUILD_SHIPPING
    return false;
#else
    return Super::ShouldCreateSubsystem(Outer);
#endif
}

void UNsCheatWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    if (UNsCheatsSubsystem* const Cheats = GetCheatsSubsystem())
    {
        Cheats->RegisterProvider(this);
    }
}

void UNsCheatWorldSubsystem::Deinitialize()
{
    if (UNsCheatsSubsystem* const Cheats = GetCheatsSubsystem())
    {
        Cheats->UnregisterProvider(this);
    }

    Super::Deinitialize();
}

bool UNsCheatWorldSubsystem::IsTickable() const
{
    return false;
}

TStatId UNsCheatWorldSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UNsCheatWorldSubsystem, STATGROUP_Tickables);
}

UNsCheatsSubsystem* UNsCheatWorldSubsystem::GetCheatsSubsystem() const
{
    const UWorld* const World = GetWorld();
    const UGameInstance* const GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    return GameInstance != nullptr ? GameInstance->GetSubsystem<UNsCheatsSubsystem>() : nullptr;
}
