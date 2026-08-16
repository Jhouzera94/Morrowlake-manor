#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RepairItem.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UGameHUDWidget;
class USoundBase;

UCLASS()
class RADAR_GUN_API ARepairItem : public AActor
{
    GENERATED_BODY()

public:
    ARepairItem();

    UFUNCTION(BlueprintCallable, Category = "Repair Item")
    bool IsCollected() const;

    UFUNCTION(BlueprintCallable, Category = "Repair Item")
    FName GetItemID() const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Repair Item")
    UStaticMeshComponent* ItemMesh;

    UPROPERTY(VisibleAnywhere, Category = "Repair Item")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, Category = "Repair Item")
    FName ItemID;

    UPROPERTY(EditAnywhere, Category = "Repair Item|Sound")
    USoundBase* PickupSound = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Repair Item")
    bool bCollected;

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    UFUNCTION()
    void CollectItem();
    void FindGameHUDWidget();
    FText GetPickupPrompt() const;

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
};
