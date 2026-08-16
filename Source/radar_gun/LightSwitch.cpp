#include "LightSwitch.h"

#include "GameHUDWidget.h"
#include "PoweredLight.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ALightSwitch::ALightSwitch()
{
    PrimaryActorTick.bCanEverTick = false;

    SwitchMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchMesh"));

    RootComponent = SwitchMesh;

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void ALightSwitch::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &ALightSwitch::UseSwitch);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &ALightSwitch::UseSwitch);

            InputComponent->KeyBindings.Last().bConsumeInput = false;
        }

        DisableInput(PlayerController);
    }

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &ALightSwitch::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &ALightSwitch::OnPlayerLeaveRange);

    FindGameHUDWidget();
}

void ALightSwitch::UseSwitch()
{
    bSwitchOn = !bSwitchOn;

    USoundBase* SoundToPlay =
        bSwitchOn ? SwitchOnSound : SwitchOffSound;

    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            SoundToPlay,
            GetActorLocation());
    }

    for (APoweredLight* Light : ControlledLights)
    {
        if (Light)
        {
            Light->ToggleSwitch();
        }
    }

    ShowInteractionPrompt();
}

void ALightSwitch::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController &&
        OtherActor == PlayerController->GetPawn())
    {
        EnableInput(PlayerController);
        ShowInteractionPrompt();
    }
}

void ALightSwitch::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController &&
        OtherActor == PlayerController->GetPawn())
    {
        DisableInput(PlayerController);
        HideInteractionPrompt();
    }
}

void ALightSwitch::FindGameHUDWidget()
{
    if (GameHUDWidget)
    {
        return;
    }

    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    if (FoundWidgets.Num() > 0)
    {
        GameHUDWidget = Cast<UGameHUDWidget>(FoundWidgets[0]);
    }
}

void ALightSwitch::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPromptWithText(
            FText::FromString(
                bSwitchOn
                    ? TEXT("[E] TURN LIGHT OFF")
                    : TEXT("[E] TURN LIGHT ON")));
    }
}

void ALightSwitch::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}
