#include "TowerCodeEntryWidget.h"

#include "EnergyTower.h"
#include "TimePortal.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"

void UTowerCodeEntryWidget::SetTowerReference(AEnergyTower* InTower)
{
    TowerRef = InTower;
    PortalRef = nullptr;
}

void UTowerCodeEntryWidget::SetPortalReference(ATimePortal* InPortal)
{
    PortalRef = InPortal;
    TowerRef = nullptr;
}

void UTowerCodeEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("TowerCodePanel"));
    WidgetTree->RootWidget = Root;

    UImage* PanelImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("ControlPanelImage"));
    static const FSoftObjectPath PanelImagePath(TEXT("/Game/Abandoned_MAnsion/tool/tower/Textures/T_HUD_EnergyTowerControlPanel.T_HUD_EnergyTowerControlPanel"));
    if (UTexture2D* PanelTexture = Cast<UTexture2D>(PanelImagePath.TryLoad()))
    {
        PanelImage->SetBrushFromTexture(PanelTexture, true);
    }
    Root->AddChildToOverlay(PanelImage);

    UVerticalBox* CodeControls = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CodeControls"));
    UOverlaySlot* CodeControlsSlot = Root->AddChildToOverlay(CodeControls);
    CodeControlsSlot->SetHorizontalAlignment(HAlign_Center);
    CodeControlsSlot->SetVerticalAlignment(VAlign_Bottom);

    UTextBlock* HeadingText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("CodeHeading"));
    HeadingText->SetText(FText::FromString(
        PortalRef ? TEXT("ENTER DESTINATION DATE") : TEXT("ENTER ACCESS CODE")));
    CodeControls->AddChildToVerticalBox(HeadingText);

    CodeTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("CodeTextBox"));
    CodeTextBox->SetHintText(PortalRef
        ? FText::FromString(TEXT("YYYY-MM-DD"))
        : FText::GetEmpty());
    CodeControls->AddChildToVerticalBox(CodeTextBox);

    SubmitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SubmitButton"));
    UTextBlock* ButtonText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubmitText"));
    ButtonText->SetText(FText::FromString(TEXT("SUBMIT")));
    SubmitButton->AddChild(ButtonText);
    CodeControls->AddChildToVerticalBox(SubmitButton);
    SubmitButton->OnClicked.AddDynamic(this, &UTowerCodeEntryWidget::SubmitCode);

    ResultText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ResultText"));
    ResultText->SetText(FText::FromString(TEXT("")));
    CodeControls->AddChildToVerticalBox(ResultText);

    if (APlayerController* Controller = GetOwningPlayer())
    {
        Controller->SetInputMode(FInputModeGameAndUI());
        Controller->bShowMouseCursor = true;
    }

    SetKeyboardFocus();
    if (CodeTextBox)
    {
        CodeTextBox->SetKeyboardFocus();
    }
}

FReply UTowerCodeEntryWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter)
    {
        SubmitCode();
        return FReply::Handled();
    }

    if (InKeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right)
    {
        ClosePanel();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UTowerCodeEntryWidget::SubmitCode()
{
    if ((!TowerRef && !PortalRef) || !CodeTextBox)
    {
        return;
    }

    const bool bAccepted = PortalRef
        ? PortalRef->SubmitDateCode(CodeTextBox->GetText().ToString())
        : TowerRef->SubmitAccessCode(CodeTextBox->GetText().ToString());

    if (bAccepted)
    {
        if (ResultText)
        {
            ResultText->SetText(FText::FromString(TEXT("ACCESS GRANTED — TOWER ACTIVATING")));
        }
        ClosePanel();
        return;
    }

    if (ResultText)
    {
        ResultText->SetText(FText::FromString(TEXT("INVALID ACCESS CODE")));
    }
}

void UTowerCodeEntryWidget::ClosePanel()
{
    RemoveFromParent();
    if (APlayerController* Controller = GetOwningPlayer())
    {
        Controller->SetInputMode(FInputModeGameOnly());
        Controller->bShowMouseCursor = false;
    }
}
