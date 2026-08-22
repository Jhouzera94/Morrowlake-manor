#include "DestinationTerminal.h"

#include "DestinationYearWidget.h"
#include "GameHUDWidget.h"
#include "TimePortal.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

ADestinationTerminal::ADestinationTerminal()
{
    // Interaction is handled by the input bindings installed while the
    // player overlaps the trigger; polling the same keys every frame was
    // redundant and could invoke the action twice.
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalMesh"));
    TerminalMesh->SetupAttachment(SceneRoot);

    MonitorPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MonitorPlane"));
    MonitorPlane->SetupAttachment(SceneRoot);
    MonitorPlane->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MonitorPlane->SetRelativeLocation(FVector(0.0f, 0.0f, 135.0f));
    MonitorPlane->SetRelativeScale3D(FVector(1.6f, 0.9f, 1.0f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(
        TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneAsset.Succeeded())
    {
        MonitorPlane->SetStaticMesh(PlaneAsset.Object);
    }

    static ConstructorHelpers::FObjectFinder<UMaterialInterface> MonitorMaterial(
        TEXT("/Game/Abandoned_MAnsion/tool/Portal/DestinationTerminal/"
             "M_TemporalDestinationMonitor_1950.M_TemporalDestinationMonitor_1950"));
    if (MonitorMaterial.Succeeded())
    {
        MonitorPlane->SetMaterial(0, MonitorMaterial.Object);
    }

    DestinationWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(
        TEXT("DestinationEntryWidget"));
    DestinationWidgetComponent->SetupAttachment(SceneRoot);
    DestinationWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    DestinationWidgetComponent->SetDrawSize(FVector2D(900.0f, 500.0f));
    DestinationWidgetComponent->SetRelativeLocation(FVector(2.0f, 0.0f, 135.0f));
    DestinationWidgetComponent->SetTwoSided(true);
    DestinationWidgetComponent->SetWidgetClass(UDestinationYearWidget::StaticClass());
    DestinationWidgetComponent->SetVisibility(false);

    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(SceneRoot);
    InteractionTrigger->SetBoxExtent(FVector(130.0f, 130.0f, 150.0f));
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionTrigger->SetGenerateOverlapEvents(true);
}

void ADestinationTerminal::BeginPlay()
{
    Super::BeginPlay();
    FindPortal();

    DestinationWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
    DestinationWidgetComponent->SetHiddenInGame(false, true);
    DestinationWidgetComponent->SetVisibility(false, true);

    if (APlayerController* PC = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr)
    {
        DestinationWidgetComponent->SetOwnerPlayer(PC->GetLocalPlayer());
        DestinationWidgetComponent->InitWidget();
    }

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &ADestinationTerminal::OnPlayerEnterRange);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this, &ADestinationTerminal::OnPlayerLeaveRange);

    if (UDestinationYearWidget* Widget = Cast<UDestinationYearWidget>(
        DestinationWidgetComponent->GetUserWidgetObject()))
    {
        Widget->SetTerminalReference(this);
    }
}

void ADestinationTerminal::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;
    if (bPlayerNearby && PC &&
        (PC->WasInputKeyJustPressed(EKeys::E) ||
         PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Left)))
    {
        OpenDestinationWidget();
    }
}

void ADestinationTerminal::FindPortal()
{
    if (!PortalReference)
    {
        PortalReference = Cast<ATimePortal>(UGameplayStatics::GetActorOfClass(
            this, ATimePortal::StaticClass()));
    }
}

void ADestinationTerminal::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || OtherActor != PC->GetPawn())
    {
        return;
    }

    bPlayerNearby = true;
    EnableInput(PC);
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::E, IE_Pressed,
            this, &ADestinationTerminal::HandleInteract);
        InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed,
            this, &ADestinationTerminal::HandleInteract);
    }

    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, Widgets, UGameHUDWidget::StaticClass(), false);
    for (UUserWidget* Found : Widgets)
    {
        if (UGameHUDWidget* HUD = Cast<UGameHUDWidget>(Found))
        {
            HUD->ShowInteractionPromptWithText(
                FText::FromString(TEXT("[E / X] ENTER DESTINATION YEAR")));
        }
    }
}

void ADestinationTerminal::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || OtherActor != PC->GetPawn())
    {
        return;
    }
    bPlayerNearby = false;
    CloseDestinationWidget();
    DisableInput(PC);
}

void ADestinationTerminal::HandleInteract()
{
    if (bPlayerNearby)
    {
        OpenDestinationWidget();
    }
}

void ADestinationTerminal::OpenDestinationWidget()
{
    FindPortal();
    if (!PortalReference || PortalReference->PortalState != EPortalState::AwaitingCode)
    {
        return;
    }

    APlayerController* PC = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;
    if (!PC)
    {
        return;
    }

    if (!DestinationScreenWidget)
    {
        DestinationScreenWidget = CreateWidget<UDestinationYearWidget>(
            PC, UDestinationYearWidget::StaticClass());
    }

    if (!DestinationScreenWidget)
    {
        return;
    }

    DestinationWidgetComponent->SetVisibility(false, true);
    DestinationScreenWidget->SetTerminalReference(this);
    if (!DestinationScreenWidget->IsInViewport())
    {
        DestinationScreenWidget->AddToViewport(100);
    }
    DestinationScreenWidget->ActivateEntry();
}

void ADestinationTerminal::CloseDestinationWidget()
{
    DestinationWidgetComponent->SetVisibility(false);
    if (DestinationScreenWidget)
    {
        DestinationScreenWidget->RemoveFromParent();
        DestinationScreenWidget = nullptr;
    }
    if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = false;
    }
}

bool ADestinationTerminal::SubmitDestinationYear(const FString& Year)
{
    FindPortal();
    if (!PortalReference || Year.TrimStartAndEnd() != RequiredYear)
    {
        return false;
    }

    const bool bAccepted = PortalReference->SubmitDateCode(RequiredYear);
    if (bAccepted)
    {
        CloseDestinationWidget();
    }
    return bAccepted;
}
