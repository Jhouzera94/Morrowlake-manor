#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MaintenanceToolPickup.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API AMaintenanceToolPickup : public AActor
{
    GENERATED_BODY()

public:
    AMaintenanceToolPickup();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Maintenance Tool",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* ToolMesh;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Maintenance Tool",
        meta = (AllowPrivateAccess = "true"))
    UBoxComponent* InteractionTrigger;

    UPROPERTY(
        EditAnywhere,
        Category = "Maintenance Tool|Sound")
    USoundBase* PickupSound = nullptr;

    UPROPERTY(
        EditAnywhere,
        Category = "Maintenance Tool|Objective")
    FText ObjectiveAfterPickup =
        FText::FromString(TEXT("Open the electrical panel"));

    UPROPERTY(
        EditAnywhere,
        Category = "Maintenance Tool|Objective")
    FText ObjectiveHintAfterPickup =
        FText::FromString(
            TEXT("Find the secured electrical panel in the manor"));

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

    void CollectTool();
    void FindGameHUDWidget();
    void ShowInteractionPrompt();
    void HideInteractionPrompt();

    bool bPlayerIsNearby = false;
    bool bCollected = false;
};