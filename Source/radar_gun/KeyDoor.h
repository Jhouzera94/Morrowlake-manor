#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KeyDoor.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class UPlayerInventoryComponent;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API AKeyDoor : public AActor
{
    GENERATED_BODY()

public:
    AKeyDoor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    UStaticMeshComponent* DoorFrameMesh;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    USceneComponent* LeftDoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    UStaticMeshComponent* LeftDoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    USceneComponent* RightDoorPivot;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    UStaticMeshComponent* RightDoorMesh;

    UPROPERTY(VisibleAnywhere, Category = "Key Door")
    UBoxComponent* InteractionTrigger;

    /*
     * Leave this false for a single door.
     * Enable it for a double door.
     */
    UPROPERTY(EditAnywhere, Category = "Key Door|Configuration")
    bool bDoubleDoor = false;

    /*
     * Must exactly match the Key ID used by its KeyPickup.
     * Example: OfficeKey
     */
    UPROPERTY(EditAnywhere, Category = "Key Door|Configuration")
    FName RequiredKeyID = TEXT("OfficeKey");

    UPROPERTY(EditAnywhere, Category = "Key Door|Configuration")
    FText RequiredKeyDisplayName =
        FText::FromString(TEXT("Office Key"));

    UPROPERTY(EditAnywhere, Category = "Key Door|Configuration")
    FText DoorDisplayName =
        FText::FromString(TEXT("Office Door"));

    UPROPERTY(EditAnywhere, Category = "Key Door|Movement")
    FRotator LeftOpenRotation =
        FRotator(0.0f, -100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Key Door|Movement")
    FRotator RightOpenRotation =
        FRotator(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, Category = "Key Door|Movement")
    float MovementDuration = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Key Door|Sound")
    USoundBase* LockedSound;

    UPROPERTY(EditAnywhere, Category = "Key Door|Sound")
    USoundBase* UnlockSound;

    UPROPERTY(EditAnywhere, Category = "Key Door|Sound")
    USoundBase* OpenSound;

    UPROPERTY(EditAnywhere, Category = "Key Door|Sound")
    USoundBase* CloseSound;

    UPROPERTY()
    UGameHUDWidget* GameHUD;

    UPROPERTY()
    UPlayerInventoryComponent* PlayerInventory;

    FRotator LeftClosedRotation;
    FRotator RightClosedRotation;

    FRotator LeftMovementStartRotation;
    FRotator RightMovementStartRotation;

    FRotator LeftTargetRotation;
    FRotator RightTargetRotation;

    float MovementElapsedTime = 0.0f;

    bool bPlayerIsNearby = false;
    bool bUnlocked = false;
    bool bDoorIsOpen = false;
    bool bDoorIsMoving = false;
    bool bOpeningDoor = false;

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
    void UnlockDoor();
    void StartDoorMovement(bool bShouldOpen);
    void UpdateDoorMovement(float DeltaTime);
    void UpdateInteractionPrompt();

    void FindGameHUD();
    void FindPlayerInventory();

    bool PlayerHasRequiredKey() const;
};