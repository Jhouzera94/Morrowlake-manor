#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyTower.generated.h"

class ATimePortal;
class APhaseRegulatorMachine;
class UAudioComponent;
class UButton;
class UBoxComponent;
class UGameHUDWidget;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UMaterialParameterCollection;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;
class UUserWidget;
class UEditableTextBox;

UCLASS(Blueprintable)
class RADAR_GUN_API AEnergyTower : public AActor
{
    GENERATED_BODY()

public:
    AEnergyTower();

    UFUNCTION(BlueprintCallable, Category = "Energy Tower")
    void StartCharging();

    UFUNCTION(BlueprintCallable, Category = "Energy Tower")
    void ResetTower();

    UFUNCTION(BlueprintPure, Category = "Energy Tower")
    float GetChargeProgress() const
    {
        return ChargeProgress;
    }

    UFUNCTION(BlueprintPure, Category = "Energy Tower")
    bool IsTowerCharged() const
    {
        return bTowerCharged;
    }

    UFUNCTION(BlueprintPure, Category = "Energy Tower")
    bool IsCharging() const
    {
        return bIsCharging;
    }

    // =========================================================
    // PUZZLE
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Energy Tower|Puzzle")
    void InstallComponent1();

    UFUNCTION(BlueprintCallable, Category = "Energy Tower|Puzzle")
    void InstallComponent2();

    UFUNCTION(BlueprintCallable, Category = "Energy Tower|Puzzle")
    bool TryInstallComponent1FromPlayer();

    UFUNCTION(BlueprintCallable, Category = "Energy Tower|Puzzle")
    bool SubmitAccessCode(const FString& Code);

    UFUNCTION(BlueprintPure, Category = "Energy Tower|Puzzle")
    bool IsComponent1Installed() const
    {
        return bComponent1Installed;
    }

    UFUNCTION(BlueprintPure, Category = "Energy Tower|Puzzle")
    bool IsComponent2Installed() const
    {
        return bComponent2Installed;
    }

    UFUNCTION(BlueprintPure, Category = "Energy Tower|Puzzle")
    bool IsAccessCodeAccepted() const
    {
        return bAccessCodeAccepted;
    }


    // =========================================================
    // TOWER CABLE
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Energy Tower|Cable")
    bool TryInstallTowerCableFromPlayer();

    UFUNCTION(BlueprintPure, Category = "Energy Tower|Cable")
    bool IsTowerCableInstalled() const
    {
        return bTowerCableInstalled;
    }


protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;


    // =========================================================
    // COMPONENTS
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower"
    )
    USceneComponent* SceneRoot;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower"
    )
    UStaticMeshComponent* TowerMesh;


    // =========================================================
    // CABLE
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower|Cable"
    )
    UStaticMeshComponent* InstalledCableMesh;

    /** Assign the Calibration Reader mesh in BP_EnergyTower.  This component
     * always exists, preventing a missing-reference runtime error. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Tower|Components")
    UStaticMeshComponent* InstalledCalibrationReaderMesh;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower|Cable"
    )
    UBoxComponent* CableInstallTrigger;


    // =========================================================
    // PORTAL
    // =========================================================

    UPROPERTY(
        EditInstanceOnly,
        BlueprintReadWrite,
        Category = "Energy Tower|Portal"
    )
    ATimePortal* ConnectedPortal = nullptr;

    /** Assign the placed Phase Regulator here.  If left blank, the tower
     * finds the first placed regulator automatically. */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Energy Tower|Requirements")
    APhaseRegulatorMachine* RequiredPhaseRegulator = nullptr;

    /** The user's full-screen code panel. Expected widget names: code, apply. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Energy Tower|Code")
    TSubclassOf<UUserWidget> TowerCodeWidgetClass;


    // =========================================================
    // MATERIAL PARAMETER COLLECTION
    // =========================================================

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Power"
    )
    UMaterialParameterCollection* PowerCollection = nullptr;


    // =========================================================
    // AUDIO
    // =========================================================

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower|Audio"
    )
    UAudioComponent* ChargingAudio;

    UPROPERTY(
        VisibleAnywhere,
        BlueprintReadOnly,
        Category = "Energy Tower|Audio"
    )
    UAudioComponent* StableEnergyAudio;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Audio"
    )
    USoundBase* ChargingLoopSound = nullptr;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Audio"
    )
    USoundBase* ChargeCompleteSound = nullptr;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Audio"
    )
    USoundBase* StableLoopSound = nullptr;


    // =========================================================
    // CHARGING
    // =========================================================

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Charging",
        meta = (ClampMin = "1.0")
    )
    float ChargingDuration = 30.0f;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Charging",
        meta = (ClampMin = "0")
    )
    int32 ScreenMaterialElement = 0;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadWrite,
        Category = "Energy Tower|Charging",
        meta = (ClampMin = "0")
    )
    int32 EnergyCylinderMaterialElement = 55;


private:

    // =========================================================
    // RUNTIME STATE
    // =========================================================

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* ScreenMaterial = nullptr;

    UPROPERTY(Transient)
    UMaterialInstanceDynamic* EnergyCylinderMaterial = nullptr;

    UPROPERTY()
    UMaterialInterface* EnergyCylinderFallbackMaterial = nullptr;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Charging"
    )
    float ChargeProgress = 0.0f;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Charging"
    )
    bool bIsCharging = false;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Charging"
    )
    bool bTowerCharged = false;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Puzzle"
    )
    bool bComponent1Installed = false;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Puzzle"
    )
    bool bComponent2Installed = false;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Puzzle"
    )
    bool bAccessCodeAccepted = false;

    UPROPERTY(
        VisibleInstanceOnly,
        Category = "Energy Tower|Cable"
    )
    bool bTowerCableInstalled = false;

    bool bPlayerNearCableInstall = false;


    // =========================================================
    // HUD
    // =========================================================

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    UPROPERTY(Transient)
    UUserWidget* ActiveTowerCodeWidget = nullptr;

    UPROPERTY(Transient)
    UEditableTextBox* TowerCodeTextBox = nullptr;

    UPROPERTY(Transient)
    UButton* TowerCodeApplyButton = nullptr;


    // =========================================================
    // CABLE INTERACTION
    // =========================================================

    UFUNCTION()
    void OnCableInstallEnterRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION()
    void OnCableInstallLeaveRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex
    );

    void HandleCableInteract();

    void FindGameHUDWidget();

    void UpdateCableInteractionPrompt();

    void UpdateTowerObjective();
    bool IsPhaseRegulatorReady() const;
    void ShowTowerCodeWidget();
    void HideTowerCodeWidget();
    void CloseTowerCodePanel();

    UFUNCTION()
    void SubmitTowerCodeFromWidget();


    // =========================================================
    // INTERNAL
    // =========================================================

    void ApplyChargeVisuals();

    void FinishCharging();
};
