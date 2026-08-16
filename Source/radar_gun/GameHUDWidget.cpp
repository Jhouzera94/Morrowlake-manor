#include "GameHUDWidget.h"

#include "PlayerInventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "GameFramework/PlayerController.h"

void UGameHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    HideInteractionPrompt();

    UpdateRepairItemIcons(false, false, false);

    EnsurePhaseRegulatorProgressWidgets();
    HidePhaseRegulatorStartupProgress();

    if (APlayerController* PlayerController = GetOwningPlayer())
    {
        if (APawn* PlayerPawn = PlayerController->GetPawn())
        {
            if (UPlayerInventoryComponent* Inventory =
                    PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>())
            {
                Inventory->RefreshPhaseRegulatorObjective();
            }
        }
    }
}

void UGameHUDWidget::ShowInteractionPrompt()
{
    if (InteractionFrameImage)
    {
        InteractionFrameImage->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }

    if (InteractionText)
    {
        InteractionText->SetText(
            FText::FromString(
                TEXT("[E] INTERACT")));

        InteractionText->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }
}

void UGameHUDWidget::HideInteractionPrompt()
{
    if (InteractionFrameImage)
    {
        InteractionFrameImage->SetVisibility(
            ESlateVisibility::Collapsed);
    }

    if (InteractionText)
    {
        InteractionText->SetVisibility(
            ESlateVisibility::Collapsed);
    }
}

void UGameHUDWidget::UpdateKeyCount(int32 CurrentKeys)
{
    if (KeyCountText)
    {
        KeyCountText->SetText(FText::AsNumber(CurrentKeys));
    }
}

void UGameHUDWidget::UpdateBlueprintCount(
    int32 CurrentBlueprints,
    int32 TotalBlueprints)
{
    if (HUDBlueprintCountText)
    {
        HUDBlueprintCountText->SetText(
            FText::Format(
                FText::FromString(TEXT("{0}/{1}")),
                FText::AsNumber(CurrentBlueprints),
                FText::AsNumber(TotalBlueprints)));
    }
}

void UGameHUDWidget::UpdateComponentCount(
    int32 CurrentComponents,
    int32 TotalComponents)
{
    if (ComponentsCountText)
    {
        ComponentsCountText->SetText(
            FText::Format(
                FText::FromString(TEXT("{0}/{1}")),
                FText::AsNumber(CurrentComponents),
                FText::AsNumber(TotalComponents)));
    }
}

void UGameHUDWidget::UpdateObjective(
    const FText& NewObjective,
    const FText& NewHint)
{
    if (CurrentObjectiveText)
    {
        CurrentObjectiveText->SetText(NewObjective);
    }

    if (ObjectiveHintText)
    {
        ObjectiveHintText->SetText(NewHint);
    }
}
void UGameHUDWidget::ShowFlashlightHint()
{
    if (FlashlightHintText)
    {
        FlashlightHintText->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }
}

void UGameHUDWidget::ShowInteractionPromptWithText(
    const FText& PromptText)
{
    if (InteractionText)
    {
        InteractionText->SetText(PromptText);

        InteractionText->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }

    if (InteractionFrameImage)
    {
        InteractionFrameImage->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }
}

void UGameHUDWidget::UpdateRepairItemIcons(
    bool bHasCalibrationReader,
    bool bHasPressureCanister,
    bool bHasResonanceConduit)
{
    if (CalibrationReaderIcon)
    {
        CalibrationReaderIcon->SetVisibility(
            bHasCalibrationReader
                ? ESlateVisibility::HitTestInvisible
                : ESlateVisibility::Collapsed);
    }

    if (PressureCanisterIcon)
    {
        PressureCanisterIcon->SetVisibility(
            bHasPressureCanister
                ? ESlateVisibility::HitTestInvisible
                : ESlateVisibility::Collapsed);
    }

    if (ResonanceConduitIcon)
    {
        ResonanceConduitIcon->SetVisibility(
            bHasResonanceConduit
                ? ESlateVisibility::HitTestInvisible
                : ESlateVisibility::Collapsed);
    }
}

void UGameHUDWidget::EnsurePhaseRegulatorProgressWidgets()
{
    if (PhaseRegulatorProgressBar && PhaseRegulatorProgressText)
    {
        PhaseRegulatorProgressContainer =
            Cast<UBorder>(PhaseRegulatorProgressBar->GetParent());
        return;
    }

    if (!WidgetTree || !WidgetTree->RootWidget)
    {
        return;
    }

    UPanelWidget* RootPanel = Cast<UPanelWidget>(WidgetTree->RootWidget);
    if (!RootPanel)
    {
        return;
    }

    PhaseRegulatorProgressContainer =
        WidgetTree->ConstructWidget<UBorder>(
            UBorder::StaticClass(),
            TEXT("RuntimePhaseRegulatorProgressContainer"));

    UVerticalBox* ProgressLayout =
        WidgetTree->ConstructWidget<UVerticalBox>(
            UVerticalBox::StaticClass(),
            TEXT("RuntimePhaseRegulatorProgressLayout"));

    PhaseRegulatorProgressText =
        WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(),
            TEXT("RuntimePhaseRegulatorProgressText"));

    PhaseRegulatorProgressBar =
        WidgetTree->ConstructWidget<UProgressBar>(
            UProgressBar::StaticClass(),
            TEXT("RuntimePhaseRegulatorProgressBar"));

    if (!PhaseRegulatorProgressContainer ||
        !ProgressLayout ||
        !PhaseRegulatorProgressText ||
        !PhaseRegulatorProgressBar)
    {
        return;
    }

    PhaseRegulatorProgressContainer->SetBrushColor(
        FLinearColor(0.015f, 0.035f, 0.045f, 0.94f));
    PhaseRegulatorProgressContainer->SetPadding(FMargin(18.0f, 12.0f));

    PhaseRegulatorProgressText->SetJustification(ETextJustify::Center);
    PhaseRegulatorProgressText->SetColorAndOpacity(
        FSlateColor(FLinearColor(0.86f, 0.79f, 0.56f, 1.0f)));

    PhaseRegulatorProgressBar->SetFillColorAndOpacity(
        FLinearColor(0.08f, 0.78f, 0.86f, 1.0f));
    PhaseRegulatorProgressBar->SetPercent(0.0f);

    ProgressLayout->AddChildToVerticalBox(PhaseRegulatorProgressText);
    if (UVerticalBoxSlot* ProgressSlot =
            ProgressLayout->AddChildToVerticalBox(PhaseRegulatorProgressBar))
    {
        ProgressSlot->SetPadding(FMargin(0.0f, 8.0f, 0.0f, 0.0f));
    }

    PhaseRegulatorProgressContainer->SetContent(ProgressLayout);

    if (UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(RootPanel))
    {
        if (UCanvasPanelSlot* CanvasSlot =
                RootCanvas->AddChildToCanvas(PhaseRegulatorProgressContainer))
        {
            CanvasSlot->SetAnchors(FAnchors(0.5f, 0.82f));
            CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
            CanvasSlot->SetPosition(FVector2D::ZeroVector);
            CanvasSlot->SetSize(FVector2D(430.0f, 86.0f));
            CanvasSlot->SetZOrder(100);
        }
    }
    else
    {
        RootPanel->AddChild(PhaseRegulatorProgressContainer);
    }
}

void UGameHUDWidget::ShowPhaseRegulatorStartupProgress(
    float Progress,
    float RemainingSeconds)
{
    EnsurePhaseRegulatorProgressWidgets();

    const float ClampedProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

    if (PhaseRegulatorProgressBar)
    {
        PhaseRegulatorProgressBar->SetPercent(ClampedProgress);
        PhaseRegulatorProgressBar->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }

    if (PhaseRegulatorProgressText)
    {
        const int32 Percent = FMath::RoundToInt(ClampedProgress * 100.0f);
        const int32 Seconds = FMath::CeilToInt(FMath::Max(0.0f, RemainingSeconds));

        PhaseRegulatorProgressText->SetText(
            FText::Format(
                FText::FromString(
                    TEXT("PHASE REGULATOR STARTING  {0}%  |  {1}s")),
                FText::AsNumber(Percent),
                FText::AsNumber(Seconds)));
        PhaseRegulatorProgressText->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }

    if (PhaseRegulatorProgressContainer)
    {
        PhaseRegulatorProgressContainer->SetVisibility(
            ESlateVisibility::HitTestInvisible);
    }
}

void UGameHUDWidget::HidePhaseRegulatorStartupProgress()
{
    if (PhaseRegulatorProgressContainer)
    {
        PhaseRegulatorProgressContainer->SetVisibility(
            ESlateVisibility::Collapsed);
    }

    if (PhaseRegulatorProgressBar && !PhaseRegulatorProgressContainer)
    {
        PhaseRegulatorProgressBar->SetVisibility(ESlateVisibility::Collapsed);
    }

    if (PhaseRegulatorProgressText && !PhaseRegulatorProgressContainer)
    {
        PhaseRegulatorProgressText->SetVisibility(ESlateVisibility::Collapsed);
    }
}
