#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

class UImage;
class UBorder;
class UProgressBar;
class UTextBlock;

UCLASS()
class RADAR_GUN_API UGameHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
    void ShowInteractionPromptWithText(
        const FText& PromptText);

    UFUNCTION(BlueprintCallable, Category = "HUD|Flashlight")
    void ShowFlashlightHint();

    UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
    void ShowInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "HUD|Interaction")
    void HideInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateKeyCount(int32 CurrentKeys);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateBlueprintCount(
        int32 CurrentBlueprints,
        int32 TotalBlueprints);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateComponentCount(
        int32 CurrentComponents,
        int32 TotalComponents);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateRepairItemIcons(
        int32 CalibrationReaderCount,
        bool bHasPressureCanister,
        bool bHasResonanceConduit,
        bool bHasTowerCable,
        bool bHasFuse,
        bool bHasScrewdriver);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateStackableItemCounts(int32 HandheldCount, int32 CableCount);

    /** Shows the currently selected repair tool in the dedicated tool slot. */
    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateActiveToolDisplay(
        bool bHasScrewdriver,
        bool bHasWrench,
        bool bShowWrench);

    UFUNCTION(BlueprintCallable, Category = "HUD|Objective")
    void UpdateObjective(
        const FText& NewObjective,
        const FText& NewHint);

    UFUNCTION(BlueprintCallable, Category = "HUD|Phase Regulator")
    void ShowPhaseRegulatorStartupProgress(
        float Progress,
        float RemainingSeconds);

    UFUNCTION(BlueprintCallable, Category = "HUD|Phase Regulator")
    void HidePhaseRegulatorStartupProgress();

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* FlashlightHintText;

    UPROPERTY(meta = (BindWidget))
    UImage* InteractionFrameImage;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* InteractionText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* KeyCountText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HUDBlueprintCountText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ComponentsCountText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* CalibrationReaderIcon;

    // Add a TextBlock with this exact name to WBP_HUD to show 2, 1, or 0 next to the reader icon.
    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* CalibrationReaderCountText;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* PressureCanisterIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ResonanceConduitIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* TowerCableIcon;

    // Optional bottom-right inventory images. Add Image widgets with these
    // exact names in WBP_HUD and assign the user's Fuse/Screwdriver textures.
    UPROPERTY(meta = (BindWidgetOptional))
    UImage* FuseIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ScrewdriverIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* PCAmountText;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* CableAmountText;

    // These are the two overlapping images in the dedicated tool slot in
    // WBP_HUD. They are found by their existing designer names at runtime.
    UPROPERTY(Transient)
    UImage* ActiveToolScrewdriverIcon = nullptr;

    UPROPERTY(Transient)
    UImage* ActiveToolWrenchIcon = nullptr;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentObjectiveText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ObjectiveHintText;

    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* PhaseRegulatorProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* PhaseRegulatorProgressText;

private:
    void EnsurePhaseRegulatorProgressWidgets();

    UPROPERTY(Transient)
    UBorder* PhaseRegulatorProgressContainer = nullptr;
};
