#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerRestorationBlueprint.generated.h"

class UBoxComponent;
class USoundBase;
class UStaticMeshComponent;
class UUserWidget;

/** A collectible fourth evidence page for the Energy Tower restoration. */
UCLASS()
class RADAR_GUN_API ATowerRestorationBlueprint : public AActor
{
    GENERATED_BODY()

public:
    ATowerRestorationBlueprint();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Schematic")
    UStaticMeshComponent* BlueprintMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Schematic")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Schematic")
    TSubclassOf<UUserWidget> EvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Schematic|Sound")
    USoundBase* PickupSound = nullptr;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnTriggerBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);
    UFUNCTION()
    void OnTriggerEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void CollectBlueprint();
    bool bPlayerIsNearby = false;
    bool bCollected = false;
};
