#pragma once

#include "CoreMinimal.h"
#include "NsCheatProviderInterface.h"
#include "Subsystems/WorldSubsystem.h"
#include "NsCheatWorldSubsystem.generated.h"

/**
 * The base of a world subsystem that adds game cheats to the NsCheats window
 * It does not exist in Shipping. It registers itself as a provider when it starts, and unregisters itself when it ends
 * A child overrides RegisterCheats_Implementation. A child that draws each frame overrides IsTickable and Tick, because the base does not tick
 */
UCLASS(Abstract)
class NSCHEATSRUNTIME_API UNsCheatWorldSubsystem : public UTickableWorldSubsystem, public INsCheatProviderInterface
{
    GENERATED_BODY()

// Functions
public:
    //~ Begin USubsystem Interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //~ End USubsystem Interface

    //~ Begin FTickableGameObject Interface
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    //~ End FTickableGameObject Interface

protected:
    /** Returns the cheats subsystem of the game instance of this world, or null */
    class UNsCheatsSubsystem* GetCheatsSubsystem() const;
};
