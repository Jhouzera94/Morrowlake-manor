// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Generator.generated.h"

class UAudioComponent;
class UBoxComponent;
class UGameHUDWidget;
class UMaterialInterface;
class UMaterialParameterCollection;
class USoundBase;
class UStaticMeshComponent;

UCLASS()
class RADAR_GUN_API AGenerator : public AActor
{
    GENERATED_BODY()

public:
    AGenerator();

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator")
    UStaticMeshComponent* GeneratorMesh;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator|Screen")
    UStaticMeshComponent* GeneratorScreenMesh;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator|Sound")
    UAudioComponent* GeneratorAudio;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator|Interaction")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generator|Screen")
    UMaterialInterface* OfflineScreenMaterial = nullptr;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Generator|Screen")
    UMaterialInterface* OnlineScreenMaterial = nullptr;

    UFUNCTION(BlueprintCallable, Category = "Generator")
    void ToggleGenerator();

    UFUNCTION(BlueprintPure, Category = "Generator")
    bool IsRunning() const;

    UFUNCTION(BlueprintPure, Category = "Generator")
    bool IsFuseInstalled() const;

protected:
    virtual void BeginPlay() override;

private:
    void InstallFuse();
    void UpdateScreenMaterial();
    void UpdatePowerEffects();

    void FindGameHUDWidget();
    void ShowInteractionPrompt();
    void HideInteractionPrompt();

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

    UPROPERTY(
        EditAnywhere,
        Category = "Generator|Power")
    UMaterialParameterCollection* PowerCollection = nullptr;

    UPROPERTY(
        EditAnywhere,
        Category = "Generator|Sound")
    USoundBase* MissingFuseSound = nullptr;

    UPROPERTY(
        EditAnywhere,
        Category = "Generator|Sound")
    USoundBase* InstallFuseSound = nullptr;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator|State",
        meta = (AllowPrivateAccess = "true"))
    bool bFuseInstalled = false;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Generator|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsRunning = false;

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    bool bPlayerIsNearby = false;
};
