#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyPickup.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API AKeyPickup : public AActor
{
    GENERATED_BODY()

public:
    AKeyPickup();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Key Pickup")
    UStaticMeshComponent* KeyMesh;

    UPROPERTY(VisibleAnywhere, Category = "Key Pickup")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, Category = "Key Pickup|Identity")
    FName KeyID = TEXT("OfficeKey");

    UPROPERTY(EditAnywhere, Category = "Key Pickup|Identity")
    FText KeyDisplayName =
        FText::FromString(TEXT("Office Key"));

    UPROPERTY(EditAnywhere, Category = "Key Pickup|Presentation")
    bool bRotateKey = true;

    UPROPERTY(EditAnywhere, Category = "Key Pickup|Presentation")
    float RotationSpeed = 45.0f;

    UPROPERTY(EditAnywhere, Category = "Key Pickup|Sound")
    USoundBase* PickupSound;

    UPROPERTY()
    UGameHUDWidget* GameHUD;

    bool bPlayerIsNearby = false;
    bool bCollected = false;

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

    void CollectKey();
    void FindGameHUD();
};