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
    void UpdateBlueprintCount(int32 CurrentBlueprints, int32 TotalBlueprints);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateComponentCount(int32 CurrentComponents, int32 TotalComponents);

    UFUNCTION(BlueprintCallable, Category = "HUD|Inventory")
    void UpdateRepairItemIcons(
        bool bHasCalibrationReader,
        bool bHasPressureCanister,
        bool bHasResonanceConduit);

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

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* PressureCanisterIcon;

    UPROPERTY(meta = (BindWidgetOptional))
    UImage* ResonanceConduitIcon;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentObjectiveText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ObjectiveHintText;

    // These can be designed directly in WBP_HUD. If they are absent, the C++
    // widget creates a compact fallback progress panel at runtime.
    UPROPERTY(meta = (BindWidgetOptional))
    UProgressBar* PhaseRegulatorProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    UTextBlock* PhaseRegulatorProgressText;

private:
    void EnsurePhaseRegulatorProgressWidgets();

    UPROPERTY(Transient)
    UBorder* PhaseRegulatorProgressContainer = nullptr;
};
