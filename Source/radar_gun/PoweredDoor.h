#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoweredDoor.generated.h"

class AGenerator;
class UBoxComponent;
class UGameHUDWidget;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API APoweredDoor : public AActor
{
    GENERATED_BODY()

public:
    APoweredDoor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Powered Door")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Powered Door")
    USceneComponent* DoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Powered Door")
    UStaticMeshComponent* DoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Powered Door")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditInstanceOnly, Category = "Powered Door|Power")
    AGenerator* PowerGenerator;

    UPROPERTY(EditAnywhere, Category = "Powered Door|Movement")
    FRotator OpenRotation =
        FRotator(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Powered Door|Movement")
    float MovementDuration = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Powered Door|Sound")
    USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, Category = "Powered Door|Sound")
    USoundBase* CloseSound;

    UPROPERTY(EditAnywhere, Category = "Powered Door|Sound")
    USoundBase* LockedSound;

    UPROPERTY()
    UGameHUDWidget* GameHUD;

    FRotator ClosedRotation;
    FRotator TargetRotation;
    FRotator MovementStartRotation;

    float MovementElapsedTime = 0.0f;

    bool bPlayerIsNearby = false;
    bool bDoorIsOpen = false;
    bool bDoorIsMoving = false;
    bool bLastPowerState = false;

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

    void UseDoor();
    void StartDoorMovement(bool bShouldOpen);
    void UpdateDoorMovement(float DeltaTime);
    void UpdateInteractionPrompt();
    void FindGameHUD();

    bool HasPower() const;
};