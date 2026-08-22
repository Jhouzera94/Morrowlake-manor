#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SlateEnums.h"
#include "DestinationYearWidget.generated.h"

class ADestinationTerminal;
class UEditableTextBox;
class UTextBlock;

UCLASS()
class RADAR_GUN_API UDestinationYearWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetTerminalReference(ADestinationTerminal* InTerminal);
    void ActivateEntry();

protected:
    virtual bool Initialize() override;
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    UPROPERTY()
    ADestinationTerminal* TerminalReference = nullptr;

    UPROPERTY()
    UEditableTextBox* YearTextBox = nullptr;

    UPROPERTY()
    UTextBlock* ResultText = nullptr;

    UFUNCTION()
    void SubmitYear();

    UFUNCTION()
    void HandleYearCommitted(
        const FText& Text, ETextCommit::Type CommitMethod);
};
