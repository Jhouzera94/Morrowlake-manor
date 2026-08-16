#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FlashlightPickup.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USpotLightComponent;
class USoundBase;
class UGameHUDWidget;
class ACharacter;

UCLASS()
class RADAR_GUN_API AFlashlightPickup : public AActor
{
    GENERATED_BODY()

public:
    AFlashlightPickup();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Flashlight")
    UStaticMeshComponent* FlashlightMesh;

    UPROPERTY(VisibleAnywhere, Category = "Flashlight")
    USpotLightComponent* FlashlightLight;

    UPROPERTY(VisibleAnywhere, Category = "Flashlight")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, Category = "Flashlight|Sound")
    USoundBase* PickupSound;

    UPROPERTY(EditAnywhere, Category = "Flashlight|Sound")
    USoundBase* ToggleSound;

    UPROPERTY(EditAnywhere, Category = "Flashlight|Aim")
    FRotator CameraAimOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, Category = "Flashlight|Aim", meta = (ClampMin = "0.0"))
    float AimRotationSpeed = 18.0f;

    UPROPERTY()
    UGameHUDWidget* GameHUD;

    UPROPERTY()
    ACharacter* OwningCharacter;

    bool bPlayerIsNearby = false;
    bool bCollected = false;
    bool bFlashlightOn = true;

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

    void CollectFlashlight();
    void ToggleFlashlight();
    void FindGameHUD();
};
