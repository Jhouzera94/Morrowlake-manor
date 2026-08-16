#include "EvidenceWidget.h"

#include "PlayerInventoryComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

void UEvidenceWidget::ConfigureFloorPlan(
    FName InFloorID,
    UTexture2D* InMapTexture,
    bool bInShowPlayerLocation)
{
    BlueprintFloorID = InFloorID;
    bShowPlayerLocation = bInShowPlayerLocation;

    if (MapImage && InMapTexture)
    {
        MapImage->SetBrushFromTexture(InMapTexture, true);
    }

    ApplyFloorPlanPresentation();
}

void UEvidenceWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CurrentZoom = 1.0f;
    CurrentPan = FVector2D::ZeroVector;
    LastMousePosition = FVector2D::ZeroVector;
    bIsDragging = false;

    if (MapImage)
    {
        MapImage->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
        MapImage->SetRenderScale(FVector2D(CurrentZoom, CurrentZoom));
        MapImage->SetRenderTranslation(CurrentPan);
    }

    ApplyFloorPlanPresentation();

    // The Phase Regulator evidence also derives from this zoomable evidence
    // widget, but the house-location marker belongs only on floor plans.
    if (GetClass()->GetName().Contains(TEXT("PhaseRegulator")))
    {
        bShowPlayerLocation = false;
    }

    CreateFallbackLocationWidgets();
    RefreshPlayerLocationMarker();
}

void UEvidenceWidget::ApplyFloorPlanPresentation()
{
    if (BlueprintFloorID != TEXT("FirstFloor") || !WidgetTree)
    {
        return;
    }

    UTextBlock* ItemTitleText = Cast<UTextBlock>(
        WidgetTree->FindWidget(TEXT("ItemTitle")));
    UTextBlock* DescriptionText = Cast<UTextBlock>(
        WidgetTree->FindWidget(TEXT("ItemDescription")));
    UTextBlock* ClueHeadingText = Cast<UTextBlock>(
        WidgetTree->FindWidget(TEXT("ItemDescription_1")));
    UTextBlock* ClueText = Cast<UTextBlock>(
        WidgetTree->FindWidget(TEXT("ItemDescription_2")));

    if (ItemTitleText)
    {
        ItemTitleText->SetText(
            FText::FromString(TEXT("FIRST FLOOR BLUEPRINT")));
    }

    if (DescriptionText)
    {
        DescriptionText->SetText(FText::FromString(
            TEXT("An architectural plan of\nMorrowlake Manor's first\nfloor. The office is marked\nfor investigation.")));
    }

    if (ClueHeadingText)
    {
        ClueHeadingText->SetText(
            FText::FromString(TEXT("NEXT LOCATION")));
    }

    if (ClueText)
    {
        ClueText->SetText(FText::FromString(
            TEXT("Search the first-floor office.\nThe Phase Regulator\nschematic may be there.")));
    }
}

void UEvidenceWidget::NativeTick(
    const FGeometry& MyGeometry,
    float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshPlayerLocationMarker();
}

FReply UEvidenceWidget::NativeOnMouseWheel(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (!MapImage)
    {
        return Super::NativeOnMouseWheel(
            InGeometry,
            InMouseEvent);
    }

    const float WheelDirection =
        InMouseEvent.GetWheelDelta();

    CurrentZoom = FMath::Clamp(
        CurrentZoom + (WheelDirection * ZoomStep),
        MinimumZoom,
        MaximumZoom);

    MapImage->SetRenderScale(
        FVector2D(CurrentZoom, CurrentZoom));

    // Return the map to the centre when fully zoomed out.
    if (CurrentZoom <= MinimumZoom)
    {
        CurrentPan = FVector2D::ZeroVector;
        MapImage->SetRenderTranslation(CurrentPan);
    }

    return FReply::Handled();
}

FReply UEvidenceWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() ==
        EKeys::LeftMouseButton)
    {
        bIsDragging = true;

        LastMousePosition =
            InMouseEvent.GetScreenSpacePosition();

        return FReply::Handled().CaptureMouse(
            TakeWidget());
    }

    return Super::NativeOnMouseButtonDown(
        InGeometry,
        InMouseEvent);
}

FReply UEvidenceWidget::NativeOnMouseButtonUp(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() ==
        EKeys::LeftMouseButton)
    {
        bIsDragging = false;

        return FReply::Handled().ReleaseMouseCapture();
    }

    return Super::NativeOnMouseButtonUp(
        InGeometry,
        InMouseEvent);
}

FReply UEvidenceWidget::NativeOnMouseMove(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (bIsDragging &&
        MapImage &&
        CurrentZoom > MinimumZoom)
    {
        const FVector2D CurrentMousePosition =
            InMouseEvent.GetScreenSpacePosition();

        const FVector2D MouseMovement =
            CurrentMousePosition - LastMousePosition;

        CurrentPan += MouseMovement;

        MapImage->SetRenderTranslation(CurrentPan);

        LastMousePosition = CurrentMousePosition;

        return FReply::Handled();
    }

    return Super::NativeOnMouseMove(
        InGeometry,
        InMouseEvent);
}

void UEvidenceWidget::CreateFallbackLocationWidgets()
{
    if (!bShowPlayerLocation || !WidgetTree || !WidgetTree->RootWidget)
    {
        return;
    }

    // The floor-plan widgets use different outer layouts. Attach the marker
    // to the canvas that actually contains MapImage instead of requiring the
    // entire widget's root to be a canvas.
    UPanelWidget* MapParent = MapImage ? MapImage->GetParent() : nullptr;
    while (MapParent && !RuntimeMarkerCanvas)
    {
        RuntimeMarkerCanvas = Cast<UCanvasPanel>(MapParent);
        MapParent = MapParent->GetParent();
    }

    if (!RuntimeMarkerCanvas)
    {
        RuntimeMarkerCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
    }

    if (!RuntimeMarkerCanvas)
    {
        return;
    }

    if (!MapLocationMarker)
    {
        MapLocationMarker =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass(),
                TEXT("RuntimeMapLocationMarker"));

        if (MapLocationMarker)
        {
            MapLocationMarker->SetText(FText::FromString(TEXT("^")));
            MapLocationMarker->SetColorAndOpacity(
                FSlateColor(FLinearColor(0.92f, 0.10f, 0.06f, 1.0f)));
            MapLocationMarker->SetShadowColorAndOpacity(FLinearColor::Black);
            MapLocationMarker->SetShadowOffset(FVector2D(2.0f, 2.0f));
            MapLocationMarker->SetJustification(ETextJustify::Center);

            FSlateFontInfo MarkerFont = MapLocationMarker->GetFont();
            MarkerFont.Size = 42;
            MapLocationMarker->SetFont(MarkerFont);

            RuntimeMarkerSlot =
                RuntimeMarkerCanvas->AddChildToCanvas(MapLocationMarker);
            if (RuntimeMarkerSlot)
            {
                RuntimeMarkerSlot->SetAnchors(FAnchors(0.0f));
                RuntimeMarkerSlot->SetAlignment(FVector2D(0.5f, 0.5f));
                RuntimeMarkerSlot->SetAutoSize(true);
                RuntimeMarkerSlot->SetZOrder(200);
            }
        }
    }
    else
    {
        RuntimeMarkerSlot = Cast<UCanvasPanelSlot>(MapLocationMarker->Slot);
    }

    if (!MapLocationText)
    {
        MapLocationText =
            WidgetTree->ConstructWidget<UTextBlock>(
                UTextBlock::StaticClass(),
                TEXT("RuntimeMapLocationText"));

        if (MapLocationText)
        {
            MapLocationText->SetColorAndOpacity(
                FSlateColor(FLinearColor(0.94f, 0.82f, 0.54f, 1.0f)));
            MapLocationText->SetShadowColorAndOpacity(FLinearColor::Black);
            MapLocationText->SetShadowOffset(FVector2D(1.0f, 1.0f));

            FSlateFontInfo LabelFont = MapLocationText->GetFont();
            LabelFont.Size = 20;
            MapLocationText->SetFont(LabelFont);

            RuntimeLocationTextSlot =
                RuntimeMarkerCanvas->AddChildToCanvas(MapLocationText);
            if (RuntimeLocationTextSlot)
            {
                RuntimeLocationTextSlot->SetAnchors(FAnchors(0.0f));
                RuntimeLocationTextSlot->SetAlignment(FVector2D(0.0f, 0.0f));
                RuntimeLocationTextSlot->SetPosition(FVector2D::ZeroVector);
                RuntimeLocationTextSlot->SetAutoSize(true);
                RuntimeLocationTextSlot->SetZOrder(201);
            }
        }
    }
    else
    {
        RuntimeLocationTextSlot = Cast<UCanvasPanelSlot>(MapLocationText->Slot);
    }

    if ((BlueprintFloorID == TEXT("GroundFloor") ||
         BlueprintFloorID == TEXT("FirstFloor")) &&
        !RuntimeCompassText)
    {
        RuntimeCompassText = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(), TEXT("RuntimeFloorPlanCompass"));
        if (RuntimeCompassText)
        {
            RuntimeCompassText->SetText(
                FText::FromString(TEXT("  N\nW * E\n  S")));
            RuntimeCompassText->SetJustification(ETextJustify::Center);
            RuntimeCompassText->SetColorAndOpacity(
                FSlateColor(FLinearColor(0.10f, 0.75f, 1.0f, 1.0f)));
            RuntimeCompassText->SetShadowColorAndOpacity(FLinearColor::Black);
            RuntimeCompassText->SetShadowOffset(FVector2D(1.0f, 1.0f));
            FSlateFontInfo CompassFont = RuntimeCompassText->GetFont();
            CompassFont.Size = 18;
            RuntimeCompassText->SetFont(CompassFont);

            RuntimeCompassSlot =
                RuntimeMarkerCanvas->AddChildToCanvas(RuntimeCompassText);
            if (RuntimeCompassSlot)
            {
                RuntimeCompassSlot->SetAnchors(FAnchors(0.0f));
                RuntimeCompassSlot->SetAlignment(FVector2D(1.0f, 0.0f));
                RuntimeCompassSlot->SetAutoSize(true);
                RuntimeCompassSlot->SetZOrder(202);
            }
        }
    }

    if (MapLocationMarker)
    {
        MapLocationMarker->SetVisibility(ESlateVisibility::Collapsed);
    }
    if (MapLocationText)
    {
        MapLocationText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UEvidenceWidget::RefreshPlayerLocationMarker()
{
    if (!bShowPlayerLocation || !MapLocationMarker || !MapLocationText)
    {
        return;
    }

    APlayerController* PlayerController = GetOwningPlayer();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        MapLocationMarker->SetVisibility(ESlateVisibility::Collapsed);
        MapLocationText->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    UPlayerInventoryComponent* Inventory =
        PlayerController->GetPawn()
        ->FindComponentByClass<UPlayerInventoryComponent>();

    // The ground-floor plan owns continuous player tracking once collected.
    // Room volumes may temporarily clear or replace the current floor ID, but
    // that must not make its player arrow disappear. The first-floor plan
    // keeps the existing strict floor-match behaviour.
    const bool bIsGroundFloorPlan =
        BlueprintFloorID == TEXT("GroundFloor");
    const bool bLocationMatchesThisFloor =
        Inventory &&
        ((bIsGroundFloorPlan &&
          Inventory->HasGroundFloorBlueprint() &&
          Inventory->IsPlayerOnGroundFloor()) ||
         (!bIsGroundFloorPlan &&
          Inventory->IsPlayerOnFirstFloor() &&
          Inventory->HasFirstFloorBlueprint()));

    if (!bLocationMatchesThisFloor)
    {
        MapLocationMarker->SetVisibility(ESlateVisibility::Collapsed);
        MapLocationText->SetVisibility(ESlateVisibility::Collapsed);
        return;
    }

    if (!RuntimeMarkerSlot)
    {
        RuntimeMarkerSlot = Cast<UCanvasPanelSlot>(MapLocationMarker->Slot);
    }

    if (RuntimeMarkerSlot && RuntimeMarkerCanvas && MapImage)
    {
        const FGeometry& MapGeometry = MapImage->GetCachedGeometry();
        const FVector2D MapSize = MapGeometry.GetLocalSize();
        if (MapSize.X > 1.0f && MapSize.Y > 1.0f)
        {
            const FVector2D TrackedMapPosition = bIsGroundFloorPlan
                ? Inventory->GetGroundFloorTrackedMapPosition()
                : Inventory->GetFirstFloorTrackedMapPosition();
            const FVector2D MapLocalPosition = TrackedMapPosition * MapSize;
            const FVector2D AbsolutePosition =
                MapGeometry.LocalToAbsolute(MapLocalPosition);
            const FVector2D CanvasPosition =
                RuntimeMarkerCanvas->GetCachedGeometry()
                .AbsoluteToLocal(AbsolutePosition);
            RuntimeMarkerSlot->SetPosition(CanvasPosition);
        }
    }

    const float PlayerYaw =
        PlayerController->GetPawn()->GetActorRotation().Yaw;
    const float FloorNorthYaw = MapNorthWorldYaw + 180.0f;
    MapLocationMarker->SetRenderTransformAngle(
        FMath::UnwindDegrees(PlayerYaw - FloorNorthYaw));

    MapLocationText->SetText(
        bIsGroundFloorPlan
            ? FText::FromString(TEXT("YOU ARE HERE  |  GROUND FLOOR"))
            : FText::Format(
                FText::FromString(TEXT("YOU ARE HERE  |  FIRST FLOOR  |  {0}")),
                Inventory->GetFirstFloorTrackedRoomName()));

    if (RuntimeLocationTextSlot && RuntimeMarkerCanvas && MapImage)
    {
        const FGeometry& MapGeometry = MapImage->GetCachedGeometry();
        const FVector2D BelowMapAbsolute = MapGeometry.LocalToAbsolute(
            FVector2D(0.0f, MapGeometry.GetLocalSize().Y + 12.0f));
        RuntimeLocationTextSlot->SetPosition(
            RuntimeMarkerCanvas->GetCachedGeometry()
            .AbsoluteToLocal(BelowMapAbsolute));
    }

    if (RuntimeCompassSlot && RuntimeMarkerCanvas && MapImage)
    {
        const FGeometry& MapGeometry = MapImage->GetCachedGeometry();
        const FVector2D CompassAbsolute = MapGeometry.LocalToAbsolute(
            FVector2D(MapGeometry.GetLocalSize().X - 14.0f, 14.0f));
        RuntimeCompassSlot->SetPosition(
            RuntimeMarkerCanvas->GetCachedGeometry()
            .AbsoluteToLocal(CompassAbsolute));
    }

    MapLocationMarker->SetVisibility(ESlateVisibility::HitTestInvisible);
    MapLocationText->SetVisibility(ESlateVisibility::HitTestInvisible);
}
