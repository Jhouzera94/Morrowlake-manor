#include "KeyDoor.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

AKeyDoor::AKeyDoor()
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
        FVector(150.0f, 150.0f, 150.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void AKeyDoor::BeginPlay()
{
    Super::BeginPlay();

    LeftClosedRotation =
        LeftDoorPivot->GetRelativeRotation();

    RightClosedRotation =
        RightDoorPivot->GetRelativeRotation();

    LeftTargetRotation = LeftClosedRotation;
    RightTargetRotation = RightClosedRotation;

    /*
     * A single door does not use the right side.
     */
    if (!bDoubleDoor)
    {
        RightDoorMesh->SetVisibility(false);
        RightDoorMesh->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AKeyDoor::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AKeyDoor::OnPlayerLeaveRange);

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
                &AKeyDoor::UseDoor);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &AKeyDoor::UseDoor);
        }

        DisableInput(PlayerController);
    }

    FindGameHUD();
    FindPlayerInventory();
}

void AKeyDoor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDoorIsMoving)
    {
        UpdateDoorMovement(DeltaTime);
    }
}

void AKeyDoor::FindGameHUD()
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

void AKeyDoor::FindPlayerInventory()
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    APawn* PlayerPawn =
        PlayerController->GetPawn();

    if (!PlayerPawn)
    {
        return;
    }

    PlayerInventory =
        PlayerPawn->FindComponentByClass<
        UPlayerInventoryComponent>();
}

bool AKeyDoor::PlayerHasRequiredKey() const
{
    return PlayerInventory &&
        PlayerInventory->HasKey(RequiredKeyID);
}

void AKeyDoor::OnPlayerEnterRange(
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

        EnableInput(PlayerController);

        if (!GameHUD)
        {
            FindGameHUD();
        }

        if (!PlayerInventory)
        {
            FindPlayerInventory();
        }

        UpdateInteractionPrompt();
    }
}

void AKeyDoor::OnPlayerLeaveRange(
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

void AKeyDoor::UpdateInteractionPrompt()
{
    if (!GameHUD)
    {
        return;
    }

    if (!bUnlocked)
    {
        if (PlayerHasRequiredKey())
        {
            const FText UnlockPrompt =
                FText::Format(
                    FText::FromString(
                        TEXT("[E] UNLOCK {0}")),
                    DoorDisplayName);

            GameHUD->ShowInteractionPromptWithText(
                UnlockPrompt);
        }
        else
        {
            const FText LockedPrompt =
                FText::Format(
                    FText::FromString(
                        TEXT("LOCKED � {0} REQUIRED")),
                    RequiredKeyDisplayName);

            GameHUD->ShowInteractionPromptWithText(
                LockedPrompt);
        }

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

void AKeyDoor::UseDoor()
{
    if (!bPlayerIsNearby || bDoorIsMoving)
    {
        return;
    }

    if (!bUnlocked)
    {
        if (PlayerHasRequiredKey())
        {
            UnlockDoor();
        }
        else
        {
            if (LockedSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this,
                    LockedSound,
                    GetActorLocation());
            }

            UpdateInteractionPrompt();
        }

        return;
    }

    StartDoorMovement(!bDoorIsOpen);
}

void AKeyDoor::UnlockDoor()
{
    bUnlocked = true;

    if (UnlockSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            UnlockSound,
            GetActorLocation());
    }

    UpdateInteractionPrompt();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s unlocked."),
        *DoorDisplayName.ToString());
}

void AKeyDoor::StartDoorMovement(
    bool bShouldOpen)
{
    bDoorIsMoving = true;
    bOpeningDoor = bShouldOpen;
    MovementElapsedTime = 0.0f;

    LeftMovementStartRotation =
        LeftDoorPivot->GetRelativeRotation();

    RightMovementStartRotation =
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

void AKeyDoor::UpdateDoorMovement(
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

    const FRotator NewLeftRotation =
        FMath::Lerp(
            LeftMovementStartRotation,
            LeftTargetRotation,
            SmoothAlpha);

    LeftDoorPivot->SetRelativeRotation(
        NewLeftRotation);

    if (bDoubleDoor)
    {
        const FRotator NewRightRotation =
            FMath::Lerp(
                RightMovementStartRotation,
                RightTargetRotation,
                SmoothAlpha);

        RightDoorPivot->SetRelativeRotation(
            NewRightRotation);
    }

    if (Alpha >= 1.0f)
    {
        LeftDoorPivot->SetRelativeRotation(
            LeftTargetRotation);

        if (bDoubleDoor)
        {
            RightDoorPivot->SetRelativeRotation(
                RightTargetRotation);
        }

        bDoorIsOpen = bOpeningDoor;
        bDoorIsMoving = false;

        UpdateInteractionPrompt();
    }
}