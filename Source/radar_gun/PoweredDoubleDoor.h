#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoweredDoubleDoor.generated.h"

class AGenerator;
class UBoxComponent;
class UGameHUDWidget;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API APoweredDoubleDoor : public AActor
{
    GENERATED_BODY()

public:
    APoweredDoubleDoor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    UStaticMeshComponent* DoorFrameMesh;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    USceneComponent* LeftDoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    UStaticMeshComponent* LeftDoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    USceneComponent* RightDoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    UStaticMeshComponent* RightDoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Powered Double Door")
    UBoxComponent* InteractionTrigger;

    /*
     * Assign the generator actor placed in the level.
     */
    UPROPERTY(EditInstanceOnly, Category = "Powered Double Door|Power")
    AGenerator* PowerGenerator;

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Movement")
    FRotator LeftOpenRotation =
        FRotator(0.0f, -100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Movement")
    FRotator RightOpenRotation =
        FRotator(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Movement")
    float MovementDuration = 1.2f;

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Sound")
    USoundBase* LockedSound;

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Sound")
    USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Sound")
    USoundBase* CloseSound;

    UPROPERTY(EditAnywhere, Category = "Powered Double Door|Text")
    FText DoorDisplayName =
        FText::FromString(TEXT("Exterior Door"));

    UPROPERTY()
    UGameHUDWidget* GameHUD;

    FRotator LeftClosedRotation;
    FRotator RightClosedRotation;

    FRotator LeftStartRotation;
    FRotator RightStartRotation;

    FRotator LeftTargetRotation;
    FRotator RightTargetRotation;

    float MovementElapsedTime = 0.0f;

    bool bPlayerIsNearby = false;
    bool bDoorIsOpen = false;
    bool bDoorIsMoving = false;
    bool bOpeningDoor = false;
    bool bPreviousPowerState = false;

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