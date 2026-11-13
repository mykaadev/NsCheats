// Copyright (C) 2025 nulled.softworks. All rights reserved.

#include "NsCheatPointPicker.h"

#include "Camera/PlayerCameraManager.h"
#include "Components/InputComponent.h"
#include "DebugCameraManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "NsCheatsSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogNsCheatsPicker, Log, All);

ANsCheatPointPicker::ANsCheatPointPicker()
{
}

void ANsCheatPointPicker::PlayerTick(const float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

#if ENABLE_DRAW_DEBUG
    if (!IsPicking())
    {
        return;
    }
    const UNsCheatsSettings* const Settings = GetDefault<UNsCheatsSettings>();
    FHitResult Hit;
    if (TraceViewCenter(Hit))
    {
        DrawDebugSphere(GetWorld(), Hit.ImpactPoint, Settings->PickMarkerRadius, 12, Settings->PickMarkerColor, false, 0.f, SDPG_Foreground, 2.f);
        DrawDebugLine(GetWorld(), Hit.ImpactPoint, Hit.ImpactPoint + Hit.ImpactNormal * Settings->PickMarkerRadius * 4.f, Settings->PickMarkerColor, false, 0.f, SDPG_Foreground, 2.f);
        DrawDebugString(GetWorld(), Hit.ImpactPoint + FVector(0.f, 0.f, Settings->PickMarkerRadius * 2.f), FString::Printf(TEXT("%s\nLeft click: pick. Esc: cancel"), *Hit.ImpactPoint.ToCompactString()), nullptr, Settings->PickMarkerColor, 0.f, true);
    }
#endif
}

void ANsCheatPointPicker::StartPick(APlayerController* const InOriginalController, FNsCheatPointPickedDelegate InOnPicked)
{
    OriginalController = InOriginalController;
    OnPicked = MoveTemp(InOnPicked);
}

void ANsCheatPointPicker::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Esc cancels a pick. The plain debug camera clears its selection on Esc, so CancelPick does that when no pick waits
    InputComponent->RemoveActionBinding(FName(TEXT("DebugCamera_Unselect")), IE_Pressed);
    InputComponent->BindAction(TEXT("DebugCamera_Unselect"), IE_Pressed, this, &ThisClass::CancelPick);
}

void ANsCheatPointPicker::Select(FHitResult const& Hit)
{
    if (!IsPicking())
    {
        Super::Select(Hit);
        return;
    }
    FinishPick(true, Hit.ImpactPoint);
}

void ANsCheatPointPicker::CancelPick()
{
    if (!IsPicking())
    {
        Unselect();
        return;
    }
    FinishPick(false, FVector::ZeroVector);
}

void ANsCheatPointPicker::FinishPick(const bool bPicked, const FVector& Point)
{
    // The delegate can start a new pick, so the pick ends before the delegate runs
    const FNsCheatPointPickedDelegate Delegate = OnPicked;
    OnPicked.Unbind();
    if (APlayerController* const Original = OriginalController.Get())
    {
        FDebugCameraManager::Get().DisableDebugCamera(Original);
    }
    else
    {
        UE_LOG(LogNsCheatsPicker, Error, TEXT("NsCheats point picker lost the player controller that it replaced"));
    }
    Delegate.ExecuteIfBound(bPicked, Point);
}

bool ANsCheatPointPicker::TraceViewCenter(FHitResult& OutHit) const
{
    if (PlayerCameraManager == nullptr)
    {
        return false;
    }
    const FVector Start = PlayerCameraManager->GetCameraLocation();
    const FVector End = Start + PlayerCameraManager->GetCameraRotation().Vector() * GetDefault<UNsCheatsSettings>()->PickTraceDistance;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(NsCheatPointPick), true, this);
    return GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, Params);
}
