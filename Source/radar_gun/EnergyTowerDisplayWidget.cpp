#include "EnergyTowerDisplayWidget.h"

#include "EnergyTower.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEnergyTowerDisplayWidget::SetTowerReference(
    AEnergyTower* InTower)
{
    TowerRef = InTower;

    CachedPercent = INDEX_NONE;
    CachedSystemStatus = -1;
    bHasCachedComponent1State = false;

    UpdateDisplay();
}

void UEnergyTowerDisplayWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateDisplay();
}

void UEnergyTowerDisplayWidget::UpdateDisplay()
{
    if (!TowerRef)
    {
        return;
    }

    const float ChargeProgress =
        FMath::Clamp(
            TowerRef->GetChargeProgress(),
            0.0f,
            1.0f
        );

    // -------------------------
    // Progress bar
    // -------------------------

    const int32 Percent = FMath::RoundToInt(ChargeProgress * 100.0f);

    if (PB_ChargeProgress && Percent != CachedPercent)
    {
        PB_ChargeProgress->SetPercent(ChargeProgress);
    }

    // -------------------------
    // Percentage text
    // -------------------------

    if (TXT_ChargePercent)
    {
        if (Percent != CachedPercent)
        {
            TXT_ChargePercent->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("CHARGING: %d%%"),
                    Percent
                )
            ));
        }
    }

    // -------------------------
    // Component 1
    // -------------------------

    const bool bComponent1Installed = TowerRef->IsComponent1Installed();
    if (TXT_Component1 &&
        (!bHasCachedComponent1State ||
         bComponent1Installed != bCachedComponent1Installed))
    {
        TXT_Component1->SetText(
            FText::FromString(
                bComponent1Installed
                ? TEXT("COMPONENT 01: INSTALLED")
                : TEXT("COMPONENT 01: MISSING")
            )
        );
    }

    // -------------------------
    // Component 2
    // -------------------------

    if (TXT_Component2)
    {
        TXT_Component2->SetVisibility(ESlateVisibility::Collapsed);
    }

    // -------------------------
    // Access code
    // -------------------------

    if (TXT_AccessCode)
    {
        TXT_AccessCode->SetVisibility(ESlateVisibility::Collapsed);
    }

    // -------------------------
    // System status
    // -------------------------

    int8 SystemStatus = 0;
    if (TowerRef->IsTowerCharged())
    {
        SystemStatus = 3;
    }
    else if (TowerRef->IsCharging())
    {
        SystemStatus = 2;
    }
    else if (bComponent1Installed)
    {
        SystemStatus = 1;
    }

    if (TXT_SystemStatus && SystemStatus != CachedSystemStatus)
    {
        FString Status;

        if (SystemStatus == 3)
        {
            Status = TEXT("SYSTEM: ONLINE");
        }
        else if (SystemStatus == 2)
        {
            Status = TEXT("SYSTEM: CHARGING");
        }
        else if (
            SystemStatus == 1)
        {
            Status = TEXT("SYSTEM: AWAITING CODE");
        }
        else
        {
            Status = TEXT("SYSTEM: OFFLINE");
        }

        TXT_SystemStatus->SetText(
            FText::FromString(Status)
        );
    }

    CachedPercent = Percent;
    CachedSystemStatus = SystemStatus;
    bCachedComponent1Installed = bComponent1Installed;
    bHasCachedComponent1State = true;
}
