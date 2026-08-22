#pragma once

#include "CoreMinimal.h"
#include "TimePortal.h"
#include "OpeningTube.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class RADAR_GUN_API AOpeningTube : public ATimePortal
{
    GENERATED_BODY()

public:
    AOpeningTube();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void ApplyInstalledVisualTransforms() override;
    virtual void BeginPortalTravelSequence() override;

private:

    // =========================================================
    // EXISTING TUBE COMPONENTS
    // =========================================================

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    UStaticMeshComponent* TubeMesh;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    USceneComponent* DoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    UStaticMeshComponent* DoorMesh;


    // =========================================================
    // ARRIVAL / OPENING MODE
    // =========================================================

    UPROPERTY(
        EditAnywhere,
        Category = "Opening Tube|Arrival",
        meta = (ToolTip = "If enabled, this tube behaves like an arrival tube: it locks player control, waits, opens the door, then restores control.")
    )
    bool bOpenOnBeginPlay = true;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Timing")
    float OpeningDelay = 3.5f;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Timing")
    float OpeningDuration = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Door")
    FRotator OpenDoorRotation =
        FRotator(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Sound")
    USoundBase* OpeningSound;


    // =========================================================
    // INTERNAL OPENING STATE
    // =========================================================

    FRotator ClosedDoorRotation;
    FRotator TargetDoorRotation;

    float OpeningElapsedTime = 0.0f;

    bool bDoorIsOpening = false;
    bool bDoorIsClosing = false;
    bool bSequenceFinished = false;

    FTimerHandle OpeningTimerHandle;
    FTimerHandle LightFlickerTimerHandle;
    FTimerHandle PortalTravelTimerHandle;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Travel",
        meta = (ClampMin = "0.1", UIMin = "0.1"))
    float PortalTravelDelay = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Travel",
        meta = (ClampMin = "0.03", UIMin = "0.03"))
    float LightFlickerInterval = 0.12f;

    void StartOpening();
    void FinishOpening();
    void ToggleInteriorLight();
    void FinishPortalTravelSequence();
    void SetPlayerControlEnabled(bool bEnabled);
};
