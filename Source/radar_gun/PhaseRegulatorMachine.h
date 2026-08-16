#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PhaseRegulatorMachine.generated.h"

class UBoxComponent;
class UAudioComponent;
class UGameHUDWidget;
class UMaterialInterface;
class UPlayerInventoryComponent;
class USceneComponent;
class UStaticMeshComponent;
class USoundBase;

UCLASS()
class RADAR_GUN_API APhaseRegulatorMachine : public AActor
{
    GENERATED_BODY()

public:
    APhaseRegulatorMachine();

    UFUNCTION(BlueprintPure, Category = "Phase Regulator")
    bool IsMachineOnline() const;

    UFUNCTION(BlueprintPure, Category = "Phase Regulator")
    int32 GetInstalledPartCount() const;

    UFUNCTION(BlueprintPure, Category = "Phase Regulator")
    bool IsMachineStarting() const;

    UFUNCTION(BlueprintPure, Category = "Phase Regulator")
    float GetStartupProgress() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator")
    UStaticMeshComponent* MachineMesh;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Display")
    UStaticMeshComponent* ScreenMesh;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Parts")
    UStaticMeshComponent* CalibrationReaderMesh;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Parts")
    UStaticMeshComponent* PressureCanisterMesh;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Parts")
    UStaticMeshComponent* ResonanceConduitMesh;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Interaction")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Audio")
    UAudioComponent* OperatingAudioComponent;

    /** One-shot sound played whenever a repair element is attached. */
    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Audio")
    USoundBase* PartAttachmentSound = nullptr;

    /** Assign a looping Sound Cue/MetaSound for continuous machine operation. */
    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Audio")
    USoundBase* OperatingLoopSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Display")
    UMaterialInterface* OfflineScreenMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Display")
    UMaterialInterface* BlueprintRequiredScreenMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Display")
    UMaterialInterface* PartRequiredScreenMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Display")
    UMaterialInterface* StartingScreenMaterial = nullptr;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Display")
    UMaterialInterface* OnlineScreenMaterial = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|State")
    bool bCalibrationReaderInstalled = false;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|State")
    bool bPressureCanisterInstalled = false;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|State")
    bool bResonanceConduitInstalled = false;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|State")
    bool bMachineOnline = false;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|State")
    bool bMachineStarting = false;

    UPROPERTY(EditAnywhere, Category = "Phase Regulator|Startup", meta = (ClampMin = "1.0"))
    float StartupDuration = 20.0f;

    UPROPERTY(VisibleAnywhere, Category = "Phase Regulator|Startup")
    float StartupElapsed = 0.0f;

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    bool bPlayerIsNearby = false;

    void UseMachine();
    void UpdatePartVisibility();
    void UpdateScreenMaterial();
    void UpdateObjective();
    void StartMachineStartup();
    void FinishMachineStartup();
    void UpdateStartupHUD();
    void FindGameHUDWidget();
    UPlayerInventoryComponent* FindPlayerInventory() const;
    bool IsManorPowerOn() const;

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
