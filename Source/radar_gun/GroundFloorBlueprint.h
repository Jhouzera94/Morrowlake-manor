// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GroundFloorBlueprint.generated.h"

class UBoxComponent;
class UGameHUDWidget;
class USoundBase;
class UStaticMeshComponent;
class UUserWidget;

UENUM(BlueprintType)
enum class EHouseFloorBlueprintKind : uint8
{
    GroundFloor UMETA(DisplayName = "Ground Floor"),
    FirstFloor UMETA(DisplayName = "First Floor")
};

UCLASS()
class RADAR_GUN_API AGroundFloorBlueprint : public AActor
{
    GENERATED_BODY()

public:
    AGroundFloorBlueprint();

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup")
    UStaticMeshComponent* BlueprintMesh;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup")
    TSubclassOf<UUserWidget> EvidenceWidgetClass;

    /** Select First Floor on a duplicate pickup placed near the stairs. */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup")
    EHouseFloorBlueprintKind BlueprintKind =
        EHouseFloorBlueprintKind::GroundFloor;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup")
    FText NextObjective;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Pickup|Sound")
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
