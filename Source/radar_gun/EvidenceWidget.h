#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EvidenceWidget.generated.h"

class UImage;
class UCanvasPanel;
class UCanvasPanelSlot;
class UTextBlock;
class UTexture2D;

UCLASS()
class RADAR_GUN_API UEvidenceWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Reuses the evidence UI for another floor plan. Call before AddToViewport. */
    void ConfigureFloorPlan(
        FName InFloorID,
        UTexture2D* InMapTexture,
        bool bInShowPlayerLocation = true);

protected:
    virtual void NativeConstruct() override;

    virtual void NativeTick(
        const FGeometry& MyGeometry,
        float InDeltaTime) override;

    virtual FReply NativeOnMouseWheel(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual FReply NativeOnMouseButtonUp(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    virtual FReply NativeOnMouseMove(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent) override;

    UPROPERTY(meta = (BindWidget))
    UImage* MapImage;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* MapLocationMarker;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* MapLocationText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    bool bShowPlayerLocation = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    FName BlueprintFloorID = TEXT("GroundFloor");

    /** World yaw that points toward the top of the blueprint image. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    float MapNorthWorldYaw = 0.0f;

private:
    float CurrentZoom = 1.0f;

    bool bIsDragging = false;

    FVector2D LastMousePosition = FVector2D::ZeroVector;

    FVector2D CurrentPan = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, Category = "Blueprint Zoom")
    float MinimumZoom = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Blueprint Zoom")
    float MaximumZoom = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Blueprint Zoom")
    float ZoomStep = 0.25f;

    void CreateFallbackLocationWidgets();
    void RefreshPlayerLocationMarker();
    void ApplyFloorPlanPresentation();

    UPROPERTY(Transient)
    UCanvasPanelSlot* RuntimeMarkerSlot = nullptr;

    UPROPERTY(Transient)
    UCanvasPanelSlot* RuntimeLocationTextSlot = nullptr;

    UPROPERTY(Transient)
    UCanvasPanel* RuntimeMarkerCanvas = nullptr;

    UPROPERTY(Transient)
    UTextBlock* RuntimeCompassText = nullptr;

    UPROPERTY(Transient)
    UCanvasPanelSlot* RuntimeCompassSlot = nullptr;
};
