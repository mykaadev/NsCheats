// Copyright (C) 2025 nulled.softworks. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DebugCameraController.h"
#include "NsCheatPointPicker.generated.h"

/** Called when a point pick ends: true with the picked point, or false when the person cancels */
DECLARE_DELEGATE_TwoParams(FNsCheatPointPickedDelegate, bool /*bPicked*/, const FVector& /*Point*/);

/**
 * A free camera that picks a point in the world. It flies like the debug camera, shows a marker where the view center meets the world, picks the point on a left click, and cancels on Esc
 * The game keeps running and the player keeps the pawn. UNsCheatsSubsystem::PickWorldPoint starts it. Without a pick in progress it works as the plain debug camera
 */
UCLASS(NotBlueprintable, Transient)
class NSCHEATSRUNTIME_API ANsCheatPointPicker : public ADebugCameraController
{
    GENERATED_BODY()

// Functions
public:
    /** Constructor */
    ANsCheatPointPicker();

    //~ Begin APlayerController Interface
    virtual void PlayerTick(float DeltaTime) override;
    //~ End APlayerController Interface

    /** Starts a pick for the player controller that the camera replaced. The delegate runs one time when the pick ends */
    void StartPick(class APlayerController* InOriginalController, FNsCheatPointPickedDelegate InOnPicked);

    /** True while a pick waits for a click */
    bool IsPicking() const { return OnPicked.IsBound(); }

protected:
    //~ Begin APlayerController Interface
    virtual void SetupInputComponent() override;
    //~ End APlayerController Interface

    //~ Begin ADebugCameraController Interface
    virtual void Select(FHitResult const& Hit) override;
    //~ End ADebugCameraController Interface

private:
    /** Cancels the pick on Esc, or clears the selection of the plain debug camera */
    void CancelPick();

    /** Ends the pick, gives the view back to the player, and runs the delegate */
    void FinishPick(bool bPicked, const FVector& Point);

    /** Traces from the camera along the view center. Returns true with the hit */
    bool TraceViewCenter(FHitResult& OutHit) const;

// Variables
private:
    /** The delegate of the pick in progress */
    FNsCheatPointPickedDelegate OnPicked;

    /** The player controller that the camera replaced */
    TWeakObjectPtr<class APlayerController> OriginalController;
};
