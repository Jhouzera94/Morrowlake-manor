#include "PoweredDoubleDoor.h"

#include "Generator.h"
#include "GameHUDWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

APoweredDoubleDoor::APoweredDoubleDoor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot"));

    RootComponent = SceneRoot;

    DoorFrameMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("DoorFrameMesh"));

    DoorFrameMesh->SetupAttachment(SceneRoot);

    LeftDoorPivot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("LeftDoorPivot"));

    LeftDoorPivot->SetupAttachment(SceneRoot);

    LeftDoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("LeftDoorMesh"));

    LeftDoorMesh->SetupAttachment(LeftDoorPivot);

    RightDoorPivot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("RightDoorPivot"));

    RightDoorPivot->SetupAttachment(SceneRoot);

    RightDoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("RightDoorMesh"));

    RightDoorMesh->SetupAttachment(RightDoorPivot);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(SceneRoot);

    InteractionTrigger->SetBoxExtent(
        FVector(180.0f, 180.0f, 180.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void APoweredDoubleDoor::BeginPlay()
{
    Super::BeginPlay();

    LeftClosedRotation =
        LeftDoorPivot->GetRelativeRotation();

    RightClosedRotation =
        RightDoorPivot->GetRelativeRotation();

    LeftTargetRotation = LeftClosedRotation;
    RightTargetRotation = RightClosedRotation;

    bPreviousPowerState = HasPower();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &APoweredDoubleDoor::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &APoweredDoubleDoor::OnPlayerLeaveRange);

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
                &APoweredDoubleDoor::UseDoor);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &APoweredDoubleDoor::UseDoor);
        }

        DisableInput(PlayerController);
    }

    FindGameHUD();
}

void APoweredDoubleDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDoorIsMoving)
    {
        UpdateDoorMovement(DeltaTime);
    }

    /*
     * This detects the generator being switched on while
     * the player is already standing beside the door.
     */
    if (bPlayerIsNearby)
    {
        const bool bCurrentPowerState = HasPower();

        if (bCurrentPowerState != bPreviousPowerState)
        {
            bPreviousPowerState = bCurrentPowerState;
            UpdateInteractionPrompt();
        }
    }
}

bool APoweredDoubleDoor::HasPower() const
{
    return PowerGenerator &&
        PowerGenerator->IsRunning();
}

void APoweredDoubleDoor::FindGameHUD()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this,
        FoundWidgets,
        UGameHUDWidget::StaticClass(),
        false);

    if (FoundWidgets.Num() > 0)
    {
        GameHUD =
            Cast<UGameHUDWidget>(
                FoundWidgets[0]);
    }
}

void APoweredDoubleDoor::OnPlayerEnterRange(
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
        bPlayerIsNearby = true;
        bPreviousPowerState = HasPower();

        EnableInput(PlayerController);

        if (!GameHUD)
        {
            FindGameHUD();
        }

        UpdateInteractionPrompt();
    }
}

void APoweredDoubleDoor::OnPlayerLeaveRange(
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
        bPlayerIsNearby = false;

        DisableInput(PlayerController);

        if (GameHUD)
        {
            GameHUD->HideInteractionPrompt();
        }
    }
}

void APoweredDoubleDoor::UpdateInteractionPrompt()
{
    if (!GameHUD)
    {
        return;
    }

    if (!HasPower())
    {
        GameHUD->ShowInteractionPromptWithText(
            FText::FromString(
                TEXT("DOOR LOCKED � POWER REQUIRED")));

        return;
    }

    if (bDoorIsOpen)
    {
        GameHUD->ShowInteractionPromptWithText(
            FText::FromString(
                TEXT("[E] CLOSE DOOR")));
    }
    else
    {
        const FText OpenPrompt =
            FText::Format(
                FText::FromString(
                    TEXT("[E] OPEN {0}")),
                DoorDisplayName);

        GameHUD->ShowInteractionPromptWithText(
            OpenPrompt);
    }
}

void APoweredDoubleDoor::UseDoor()
{
    if (!bPlayerIsNearby || bDoorIsMoving)
    {
        return;
    }

    if (!HasPower())
    {
        if (LockedSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                LockedSound,
                GetActorLocation());
        }

        UpdateInteractionPrompt();
        return;
    }

    StartDoorMovement(!bDoorIsOpen);
}

void APoweredDoubleDoor::StartDoorMovement(
    bool bShouldOpen)
{
    bDoorIsMoving = true;
    bOpeningDoor = bShouldOpen;
    MovementElapsedTime = 0.0f;

    LeftStartRotation =
        LeftDoorPivot->GetRelativeRotation();

    RightStartRotation =
        RightDoorPivot->GetRelativeRotation();

    if (bShouldOpen)
    {
        LeftTargetRotation =
            LeftClosedRotation + LeftOpenRotation;

        RightTargetRotation =
            RightClosedRotation + RightOpenRotation;

        if (OpenSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                OpenSound,
                GetActorLocation());
        }
    }
    else
    {
        LeftTargetRotation = LeftClosedRotation;
        RightTargetRotation = RightClosedRotation;

        if (CloseSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                CloseSound,
                GetActorLocation());
        }
    }
}

void APoweredDoubleDoor::UpdateDoorMovement(
    float DeltaTime)
{
    MovementElapsedTime += DeltaTime;

    const float Alpha =
        FMath::Clamp(
            MovementElapsedTime / MovementDuration,
            0.0f,
            1.0f);

    const float SmoothAlpha =
        FMath::InterpEaseInOut(
            0.0f,
            1.0f,
            Alpha,
            2.0f);

    LeftDoorPivot->SetRelativeRotation(
        FMath::Lerp(
            LeftStartRotation,
            LeftTargetRotation,
            SmoothAlpha));

    RightDoorPivot->SetRelativeRotation(
        FMath::Lerp(
            RightStartRotation,
            RightTargetRotation,
            SmoothAlpha));

    if (Alpha >= 1.0f)
    {
        LeftDoorPivot->SetRelativeRotation(
            LeftTargetRotation);

        RightDoorPivot->SetRelativeRotation(
            RightTargetRotation);

        bDoorIsOpen = bOpeningDoor;
        bDoorIsMoving = false;

        UpdateInteractionPrompt();
    }
}