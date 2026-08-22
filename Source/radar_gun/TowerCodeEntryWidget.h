#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerCodeEntryWidget.generated.h"

class AEnergyTower;
class ATimePortal;
class UButton;
class UEditableTextBox;
class UTextBlock;

// A self-contained UI used by the Energy Tower once all physical parts are installed.
UCLASS()
class RADAR_GUN_API UTowerCodeEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetTowerReference(AEnergyTower* InTower);
    void SetPortalReference(ATimePortal* InPortal);

protected:
    virtual void NativeConstruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    UPROPERTY()
    AEnergyTower* TowerRef = nullptr;

    UPROPERTY()
    ATimePortal* PortalRef = nullptr;

    UPROPERTY()
    UEditableTextBox* CodeTextBox = nullptr;

    UPROPERTY()
    UTextBlock* ResultText = nullptr;

    UPROPERTY()
    UButton* SubmitButton = nullptr;

    UFUNCTION()
    void SubmitCode();

    void ClosePanel();
};
