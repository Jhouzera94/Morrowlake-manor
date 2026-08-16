#include "EnergyTowerDisplayWidget.h"

#include "EnergyTower.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UEnergyTowerDisplayWidget::SetTowerReference(
    AEnergyTower* InTower)
{
    TowerRef = InTower;

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

    if (PB_ChargeProgress)
    {
        PB_ChargeProgress->SetPercent(ChargeProgress);
    }

    // -------------------------
    // Percentage text
    // -------------------------

    if (TXT_ChargePercent)
    {
        const int32 Percent =
            FMath::RoundToInt(ChargeProgress * 100.0f);

        TXT_ChargePercent->SetText(
            FText::FromString(
                FString::Printf(
                    TEXT("CHARGING: %d%%"),
                    Percent
                )
            )
        );
    }

    // -------------------------
    // Component 1
    // -------------------------

    if (TXT_Component1)
    {
        TXT_Component1->SetText(
            FText::FromString(
                TowerRef->IsComponent1Installed()
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
        TXT_Component2->SetText(
            FText::FromString(
                TowerRef->IsComponent2Installed()
                ? TEXT("COMPONENT 02: INSTALLED")
                : TEXT("COMPONENT 02: MISSING")
            )
        );
    }

    // -------------------------
    // Access code
    // -------------------------

    if (TXT_AccessCode)
    {
        TXT_AccessCode->SetText(
            FText::FromString(
                TowerRef->IsAccessCodeAccepted()
                ? TEXT("ACCESS CODE: ACCEPTED")
                : TEXT("ACCESS CODE: REQUIRED")
            )
        );
    }

    // -------------------------
    // System status
    // -------------------------

    if (TXT_SystemStatus)
    {
        FString Status;

        if (TowerRef->IsTowerCharged())
        {
            Status = TEXT("SYSTEM: ONLINE");
        }
        else if (TowerRef->IsAccessCodeAccepted())
        {
            Status = TEXT("SYSTEM: CHARGING");
        }
        else if (
            TowerRef->IsComponent1Installed() &&
            TowerRef->IsComponent2Installed())
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
}