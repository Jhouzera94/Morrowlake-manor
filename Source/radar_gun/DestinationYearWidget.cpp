#include "DestinationYearWidget.h"

#include "DestinationTerminal.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"

void UDestinationYearWidget::SetTerminalReference(
    ADestinationTerminal* InTerminal)
{
    TerminalReference = InTerminal;
}

bool UDestinationYearWidget::Initialize()
{
    if (!Super::Initialize())
    {
        return false;
    }

    if (WidgetTree->RootWidget)
    {
        return true;
    }

    SetIsFocusable(true);

    UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(
        UBorder::StaticClass(), TEXT("DestinationPanel"));
    Panel->SetBrushColor(FLinearColor(0.005f, 0.025f, 0.035f, 0.92f));
    Panel->SetPadding(FMargin(36.0f));
    WidgetTree->RootWidget = Panel;

    UVerticalBox* Controls = WidgetTree->ConstructWidget<UVerticalBox>(
        UVerticalBox::StaticClass(), TEXT("DestinationControls"));
    Panel->SetContent(Controls);

    UTextBlock* Heading = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("Heading"));
    Heading->SetText(FText::FromString(TEXT("TEMPORAL DESTINATION CONTROL")));
    Heading->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 0.85f, 1.0f)));
    Controls->AddChildToVerticalBox(Heading);

    UTextBlock* Instruction = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("Instruction"));
    Instruction->SetText(FText::FromString(TEXT("ENTER DESTINATION YEAR")));
    Instruction->SetColorAndOpacity(FSlateColor(FLinearColor(0.25f, 0.95f, 1.0f)));
    Controls->AddChildToVerticalBox(Instruction);

    YearTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(
        UEditableTextBox::StaticClass(), TEXT("YearEntry"));
    YearTextBox->SetHintText(FText::FromString(TEXT("YYYY")));
    YearTextBox->SetText(FText::GetEmpty());
    YearTextBox->OnTextCommitted.AddDynamic(
        this, &UDestinationYearWidget::HandleYearCommitted);
    Controls->AddChildToVerticalBox(YearTextBox);

    UButton* ConfirmButton = WidgetTree->ConstructWidget<UButton>(
        UButton::StaticClass(), TEXT("ConfirmDestination"));
    UTextBlock* ConfirmText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("ConfirmText"));
    ConfirmText->SetText(FText::FromString(TEXT("CONFIRM DESTINATION")));
    ConfirmButton->AddChild(ConfirmText);
    ConfirmButton->OnClicked.AddDynamic(this, &UDestinationYearWidget::SubmitYear);
    Controls->AddChildToVerticalBox(ConfirmButton);

    ResultText = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass(), TEXT("Result"));
    ResultText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.55f, 0.1f)));
    Controls->AddChildToVerticalBox(ResultText);

    return true;
}

void UDestinationYearWidget::ActivateEntry()
{
    if (APlayerController* PC = GetOwningPlayer())
    {
        FInputModeUIOnly InputMode;
        if (YearTextBox)
        {
            InputMode.SetWidgetToFocus(YearTextBox->TakeWidget());
        }
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
    if (YearTextBox)
    {
        YearTextBox->SetKeyboardFocus();
    }
}

void UDestinationYearWidget::HandleYearCommitted(
    const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod == ETextCommit::OnEnter)
    {
        SubmitYear();
    }
}

FReply UDestinationYearWidget::NativeOnKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Enter)
    {
        SubmitYear();
        return FReply::Handled();
    }
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UDestinationYearWidget::SubmitYear()
{
    if (!TerminalReference || !YearTextBox)
    {
        return;
    }

    if (TerminalReference->SubmitDestinationYear(
        YearTextBox->GetText().ToString()))
    {
        ResultText->SetText(FText::FromString(TEXT("DESTINATION 1950 CONFIRMED")));
    }
    else
    {
        ResultText->SetText(FText::FromString(TEXT("INVALID DESTINATION YEAR")));
    }
}
