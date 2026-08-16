#include "PoweredDoor.h"

#include "Generator.h"
#include "GameHUDWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

APoweredDoor::APoweredDoor()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot"));

    RootComponent = SceneRoot;

    DoorPivot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("DoorPivot"));

    DoorPivot->SetupAttachment(SceneRoot);

    DoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("DoorMesh"));

    DoorMesh->SetupAttachment(DoorPivot);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(SceneRoot);
    InteractionTrigger->SetBoxExtent(
        FVector(150.0f, 150.0f, 150.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void APoweredDoor::BeginPlay()
{
    Super::BeginPlay();

    ClosedRotation =
        DoorPivot->GetRelativeRotation();

    TargetRotation = ClosedRotation;

    bLastPowerState = HasPower();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &APoweredDoor::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &APoweredDoor::OnPlayerLeaveRange);

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
                &APoweredDoor::UseDoor);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &APoweredDoor::UseDoor);
        }

        DisableInput(PlayerController);
    }

    FindGameHUD();
}

void APoweredDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDoorIsMoving)
    {
        UpdateDoorMovement(DeltaTime);
    }

    if (bPlayerIsNearby)
    {
        const bool bCurrentPowerState = HasPower();

        if (bCurrentPowerState != bLastPowerState)
        {
            bLastPowerState = bCurrentPowerState;
            UpdateInteractionPrompt();
        }
    }
}

bool APoweredDoor::HasPower() const
{
    return PowerGenerator &&
        PowerGenerator->IsRunning();
}

void APoweredDoor::FindGameHUD()
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
            Cast<UGameHUDWidget>(FoundWidgets[0]);
    }
}

void APoweredDoor::OnPlayerEnterRange(
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
        bLastPowerState = HasPower();

        EnableInput(PlayerController);

        if (!GameHUD)
        {
            FindGameHUD();
        }

        UpdateInteractionPrompt();
    }
}

void APoweredDoor::OnPlayerLeaveRange(
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

void APoweredDoor::UpdateInteractionPrompt()
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
        GameHUD->ShowInteractionPromptWithText(
            FText::FromString(
                TEXT("[E] OPEN DOOR")));
    }
}

void APoweredDoor::UseDoor()
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

void APoweredDoor::StartDoorMovement(
    bool bShouldOpen)
{
    bDoorIsMoving = true;
    MovementElapsedTime = 0.0f;
    MovementStartRotation =
        DoorPivot->GetRelativeRotation();

    if (bShouldOpen)
    {
        TargetRotation =
            ClosedRotation + OpenRotation;

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
        TargetRotation = ClosedRotation;

        if (CloseSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                CloseSound,
                GetActorLocation());
        }
    }
}

void APoweredDoor::UpdateDoorMovement(
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

    const FRotator NewRotation =
        FMath::Lerp(
            MovementStartRotation,
            TargetRotation,
            SmoothAlpha);

    DoorPivot->SetRelativeRotation(NewRotation);

    if (Alpha >= 1.0f)
    {
        DoorPivot->SetRelativeRotation(
            TargetRotation);

        bDoorIsMoving = false;
        bDoorIsOpen = !bDoorIsOpen;

        UpdateInteractionPrompt();
    }
}