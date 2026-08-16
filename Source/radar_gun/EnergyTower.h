#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnergyTower.generated.h"

class UAudioComponent;
class UMaterialInstanceDynamic;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;

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

    // -------------------------
    // Puzzle
    // -------------------------

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

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Tower")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Tower")
    UStaticMeshComponent* TowerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Tower|Audio")
    UAudioComponent* ChargingAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Energy Tower|Audio")
    UAudioComponent* StableEnergyAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy Tower|Audio")
    USoundBase* ChargingLoopSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy Tower|Audio")
    USoundBase* ChargeCompleteSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Energy Tower|Audio")
    USoundBase* StableLoopSound = nullptr;

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

private:
    UPROPERTY(Transient)
    UMaterialInstanceDynamic* ScreenMaterial = nullptr;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Charging")
    float ChargeProgress = 0.0f;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Charging")
    bool bIsCharging = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Charging")
    bool bTowerCharged = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Puzzle")
    bool bComponent1Installed = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Puzzle")
    bool bComponent2Installed = false;

    UPROPERTY(VisibleInstanceOnly, Category = "Energy Tower|Puzzle")
    bool bAccessCodeAccepted = false;

    void ApplyChargeVisuals();
    void FinishCharging();
};