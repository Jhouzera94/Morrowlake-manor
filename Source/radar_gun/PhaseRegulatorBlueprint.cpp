#include "PhaseRegulatorBlueprint.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

APhaseRegulatorBlueprint::APhaseRegulatorBlueprint()
{
    PrimaryActorTick.bCanEverTick = false;

    BlueprintMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlueprintMesh"));
    RootComponent = BlueprintMesh;

    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void APhaseRegulatorBlueprint::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &APhaseRegulatorBlueprint::OnTriggerBegin);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this, &APhaseRegulatorBlueprint::OnTriggerEnd);

    FindGameHUDWidget();
}

void APhaseRegulatorBlueprint::FindGameHUDWidget()
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

void APhaseRegulatorBlueprint::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPrompt();
    }
}

void APhaseRegulatorBlueprint::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

void APhaseRegulatorBlueprint::OnTriggerBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>() || bCollected)
    {
        return;
    }

    bPlayerIsNearby = true;
    ShowInteractionPrompt();

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &APhaseRegulatorBlueprint::CollectBlueprint);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &APhaseRegulatorBlueprint::CollectBlueprint);
        }
    }
}

void APhaseRegulatorBlueprint::OnTriggerEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    bPlayerIsNearby = false;
    HideInteractionPrompt();

    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        DisableInput(PlayerController);
    }
}

void APhaseRegulatorBlueprint::CollectBlueprint()
{
    if (!bPlayerIsNearby || bCollected)
    {
        return;
    }

    bCollected = true;
    bPlayerIsNearby = false;
    HideInteractionPrompt();

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    }

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        DisableInput(PlayerController);

        APawn* PlayerPawn = PlayerController->GetPawn();

        if (PlayerPawn)
        {
            UPlayerInventoryComponent* Inventory =
                PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>();

            if (Inventory)
            {
                // Keep the widget assignment with the collected evidence. This
                // avoids requiring the same class to be configured separately
                // on the player's inventory component Blueprint.
                if (EvidenceWidgetClass)
                {
                    Inventory->PhaseRegulatorEvidenceWidgetClass =
                        EvidenceWidgetClass;
                }

                Inventory->AddPhaseRegulatorBlueprint();
            }
            else
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("PlayerInventoryComponent was not found while collecting Phase Regulator evidence."));
            }
        }

        if (EvidenceWidgetClass)
        {
            UUserWidget* EvidenceWidget =
                CreateWidget<UUserWidget>(PlayerController, EvidenceWidgetClass);

            if (EvidenceWidget)
            {
                EvidenceWidget->AddToViewport();

                PlayerController->bShowMouseCursor = true;

                FInputModeUIOnly InputMode;
                InputMode.SetWidgetToFocus(EvidenceWidget->TakeWidget());
                InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

                PlayerController->SetInputMode(InputMode);
            }
        }
    }

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}
