#include "ElectricalPanel.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

AElectricalPanel::AElectricalPanel()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundAsset(
        TEXT("/Engine/VREditor/Sounds/UI/Object_PickUp.Object_PickUp"));
    if (PickupSoundAsset.Succeeded())
    {
        FusePickupSound = PickupSoundAsset.Object;
    }

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

    RootComponent = SceneRoot;

    PanelMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PanelMesh"));

    PanelMesh->SetupAttachment(SceneRoot);

    DoorPivot =
        CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));

    DoorPivot->SetupAttachment(SceneRoot);

    DoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorMesh"));

    DoorMesh->SetupAttachment(DoorPivot);

    FuseMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FuseMesh"));

    FuseMesh->SetupAttachment(SceneRoot);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(SceneRoot);
    InteractionTrigger->SetBoxExtent(
        FVector(100.0f, 100.0f, 120.0f));

    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

    ToolRequiredObjective =
        FText::FromString(
            TEXT("A maintenance tool is required"));

    ToolRequiredHint =
        FText::FromString(
            TEXT("Find a tool capable of opening this panel"));

    FuseObjective =
        FText::FromString(
            TEXT("Collect the generator fuse"));

    FuseObjectiveHint =
        FText::FromString(
            TEXT("A usable fuse is inside the electrical panel"));

    InstallFuseObjective =
        FText::FromString(
            TEXT("Install the fuse in the generator"));

    InstallFuseObjectiveHint =
        FText::FromString(
            TEXT("Take the fuse to the generator house"));
}

void AElectricalPanel::BeginPlay()
{
    Super::BeginPlay();

    SetActorTickEnabled(false);

    DoorPivot->SetRelativeRotation(
        FRotator(0.0f, ClosedDoorAngle, 0.0f));

    // Visible in the Blueprint editor, hidden when gameplay begins.
    FuseMesh->SetVisibility(false);
    FuseMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AElectricalPanel::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AElectricalPanel::OnPlayerLeaveRange);

    FindGameHUDWidget();
}

void AElectricalPanel::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDoorIsMoving)
    {
        return;
    }

    const FRotator CurrentRotation =
        DoorPivot->GetRelativeRotation();

    const FRotator TargetRotation(
        0.0f,
        OpenDoorAngle,
        0.0f);

    const FRotator NewRotation =
        FMath::RInterpTo(
            CurrentRotation,
            TargetRotation,
            DeltaTime,
            DoorOpeningSpeed);

    DoorPivot->SetRelativeRotation(NewRotation);

    if (FMath::IsNearlyEqual(
        NewRotation.Yaw,
        OpenDoorAngle,
        0.5f))
    {
        DoorPivot->SetRelativeRotation(TargetRotation);
        bDoorIsMoving = false;
        SetActorTickEnabled(false);
    }
}

void AElectricalPanel::FindGameHUDWidget()
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

void AElectricalPanel::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPrompt();
    }
}

void AElectricalPanel::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

void AElectricalPanel::OnPlayerEnterRange(
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
        OtherActor != PlayerController->GetPawn())
    {
        return;
    }

    bPlayerIsNearby = true;

    if (!bFuseCollected)
    {
        ShowInteractionPrompt();
    }

    EnableInput(PlayerController);

    if (InputComponent)
    {
        InputComponent->BindKey(
            EKeys::E,
            IE_Pressed,
            this,
            &AElectricalPanel::UsePanel);

        InputComponent->BindKey(
            EKeys::Gamepad_FaceButton_Left,
            IE_Pressed,
            this,
            &AElectricalPanel::UsePanel);
    }
}

void AElectricalPanel::OnPlayerLeaveRange(
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

void AElectricalPanel::UsePanel()
{
    if (!bPlayerIsNearby || bFuseCollected)
    {
        return;
    }

    // Once open, the next press of E collects the fuse.
    if (bPanelOpen)
    {
        CollectFuse();
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
        PlayerPawn->FindComponentByClass<
        UPlayerInventoryComponent>();

    if (!Inventory || !Inventory->HasMaintenanceTool())
    {
        if (LockedSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                LockedSound,
                GetActorLocation());
        }

        FindGameHUDWidget();

        if (GameHUDWidget)
        {
            GameHUDWidget->UpdateObjective(
                ToolRequiredObjective,
                ToolRequiredHint);
        }

        return;
    }

    OpenPanel();
}

void AElectricalPanel::OpenPanel()
{
    if (bPanelOpen)
    {
        return;
    }

    bPanelOpen = true;
    bDoorIsMoving = true;

    if (OpeningSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            OpeningSound,
            GetActorLocation());
    }

    FuseMesh->SetVisibility(true);
    FuseMesh->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly);

    SetActorTickEnabled(true);

    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            FuseObjective,
            FuseObjectiveHint);

        // Keep E visible so the player knows to press it again.
        GameHUDWidget->ShowInteractionPrompt();
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Electrical panel opened. Generator fuse revealed."));
}

void AElectricalPanel::CollectFuse()
{
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
        PlayerPawn->FindComponentByClass<
        UPlayerInventoryComponent>();

    if (!Inventory)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PlayerInventoryComponent was not found."));
        return;
    }

    Inventory->AddGeneratorFuse();

    bFuseCollected = true;

    FuseMesh->SetVisibility(false);
    FuseMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    HideInteractionPrompt();
    DisableInput(PlayerController);

    if (FusePickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            FusePickupSound,
            FuseMesh->GetComponentLocation());
    }

    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            InstallFuseObjective,
            InstallFuseObjectiveHint);
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Generator fuse collected."));
}
