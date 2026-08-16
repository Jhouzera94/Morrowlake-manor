#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MansionStartupSubsystem.generated.h"

UCLASS()
class RADAR_GUN_API UMansionStartupSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

private:
    void RepairStartup();
    void UpdateControllerLook();

    int32 RemainingAttempts = 30;
    FTimerHandle RepairTimer;
    FTimerHandle ControllerLookTimer;

    float ControllerLookDeadZone = 0.35f;
    float ControllerYawSpeed = 120.0f;
    float ControllerPitchSpeed = 90.0f;
};
