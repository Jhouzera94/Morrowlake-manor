#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ElectricalPanel.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API AElectricalPanel : public AActor
{
    GENERATED_BODY()

public:
    AElectricalPanel();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    UStaticMeshComponent* PanelMesh;

    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    USceneComponent* DoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    UStaticMeshComponent* FuseMesh;

    UPROPERTY(VisibleAnywhere, Category = "Electrical Panel")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Door")
    float ClosedDoorAngle = 0.0f;

    // Keep this negative if that makes your door open outward.
    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Door")
    float OpenDoorAngle = -100.0f;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Door")
    float DoorOpeningSpeed = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Sound")
    USoundBase* LockedSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Sound")
    USoundBase* OpeningSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Sound")
    USoundBase* FusePickupSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText ToolRequiredObjective;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText ToolRequiredHint;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText FuseObjective;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText FuseObjectiveHint;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText InstallFuseObjective;

    UPROPERTY(EditAnywhere, Category = "Electrical Panel|Objective")
    FText InstallFuseObjectiveHint;

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    UFUNCTION()
    void OnPlayerEnterRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerLeaveRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void UsePanel();
    void OpenPanel();
    void CollectFuse();
    void FindGameHUDWidget();
    void ShowInteractionPrompt();
    void HideInteractionPrompt();

    bool bPlayerIsNearby = false;
    bool bPanelOpen = false;
    bool bDoorIsMoving = false;
    bool bFuseCollected = false;
};