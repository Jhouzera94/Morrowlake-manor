#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhaseRegulatorBlueprint.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class USoundBase;
class UStaticMeshComponent;
class UUserWidget;

UCLASS()
class RADAR_GUN_API APhaseRegulatorBlueprint : public AActor
{
    GENERATED_BODY()

public:
    APhaseRegulatorBlueprint();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blueprint Pickup")
    UStaticMeshComponent* BlueprintMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blueprint Pickup")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Pickup")
    TSubclassOf<UUserWidget> EvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Pickup")
    FText NextObjective;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Pickup|Sound")
    USoundBase* PickupSound = nullptr;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnTriggerBegin(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEnd(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void CollectBlueprint();
    void FindGameHUDWidget();
    void ShowInteractionPrompt();
    void HideInteractionPrompt();

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    bool bPlayerIsNearby = false;
    bool bCollected = false;
};