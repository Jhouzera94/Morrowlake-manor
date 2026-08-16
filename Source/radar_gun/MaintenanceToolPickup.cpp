#include "MaintenanceToolPickup.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AMaintenanceToolPickup::AMaintenanceToolPickup()
{
    PrimaryActorTick.bCanEverTick = false;

    ToolMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));

    RootComponent = ToolMesh;

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

    static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundAsset(
        TEXT("/Engine/VREditor/Sounds/UI/Object_PickUp.Object_PickUp"));
    if (PickupSoundAsset.Succeeded())
    {
        PickupSound = PickupSoundAsset.Object;
    }
}

void AMaintenanceToolPickup::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AMaintenanceToolPickup::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AMaintenanceToolPickup::OnPlayerLeaveRange);

    FindGameHUDWidget();
}

void AMaintenanceToolPickup::FindGameHUDWidget()
{
    if (GameHUDWidget)
    {
        return;
    }

    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this,
        FoundWidgets,
        UGameHUDWidget::StaticClass(),
        false);

    if (FoundWidgets.Num() > 0)
    {
        GameHUDWidget =
            Cast<UGameHUDWidget>(FoundWidgets[0]);
    }
}

void AMaintenanceToolPickup::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPrompt();
    }
}

void AMaintenanceToolPickup::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

void AMaintenanceToolPickup::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController ||
        OtherActor != PlayerController->GetPawn() ||
        bCollected)
    {
        return;
    }

    bPlayerIsNearby = true;
    ShowInteractionPrompt();

    EnableInput(PlayerController);

    if (InputComponent)
    {
        InputComponent->BindKey(
            EKeys::E,
            IE_Pressed,
            this,
            &AMaintenanceToolPickup::CollectTool);

        InputComponent->BindKey(
            EKeys::Gamepad_FaceButton_Left,
            IE_Pressed,
            this,
            &AMaintenanceToolPickup::CollectTool);
    }
}

void AMaintenanceToolPickup::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController ||
        OtherActor != PlayerController->GetPawn())
    {
        return;
    }

    bPlayerIsNearby = false;
    HideInteractionPrompt();
    DisableInput(PlayerController);
}

void AMaintenanceToolPickup::CollectTool()
{
    if (!bPlayerIsNearby || bCollected)
    {
        return;
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    APawn* PlayerPawn = PlayerController->GetPawn();

    if (!PlayerPawn)
    {
        return;
    }

    UPlayerInventoryComponent* Inventory =
        PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>();

    if (!Inventory)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PlayerInventoryComponent was not found on the player."));

        return;
    }

    bCollected = true;
    bPlayerIsNearby = false;

    Inventory->AddMaintenanceTool();
    HideInteractionPrompt();
    DisableInput(PlayerController);

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation());
    }

    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            ObjectiveAfterPickup,
            ObjectiveHintAfterPickup);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Maintenance Tool collected."));

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}
