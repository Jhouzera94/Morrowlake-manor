#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnergyTowerDisplayWidget.generated.h"

class AEnergyTower;
class UProgressBar;
class UTextBlock;

UCLASS()
class RADAR_GUN_API UEnergyTowerDisplayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Energy Tower")
    void SetTowerReference(AEnergyTower* InTower);

protected:
    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime
    ) override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_SystemStatus = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Component1 = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_Component2 = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_AccessCode = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TXT_ChargePercent = nullptr;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* PB_ChargeProgress = nullptr;

private:
    UPROPERTY()
    AEnergyTower* TowerRef = nullptr;

    void UpdateDisplay();
};