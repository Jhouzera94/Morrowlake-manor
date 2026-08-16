#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OpeningTube.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class RADAR_GUN_API AOpeningTube : public AActor
{
    GENERATED_BODY()

public:
    AOpeningTube();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    UStaticMeshComponent* TubeMesh;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    USceneComponent* DoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Opening Tube")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Timing")
    float OpeningDelay = 3.5f;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Timing")
    float OpeningDuration = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Door")
    FRotator OpenDoorRotation =
        FRotator(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Opening Tube|Sound")
    USoundBase* OpeningSound;

    FRotator ClosedDoorRotation;
    FRotator TargetDoorRotation;

    float OpeningElapsedTime = 0.0f;

    bool bDoorIsOpening = false;
    bool bSequenceFinished = false;

    FTimerHandle OpeningTimerHandle;

    void StartOpening();
    void FinishOpening();
    void SetPlayerControlEnabled(bool bEnabled);
};