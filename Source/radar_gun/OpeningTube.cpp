#include "OpeningTube.h"

#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"


AOpeningTube::AOpeningTube()
{
    PrimaryActorTick.bCanEverTick = true;


    // =========================================================
    // ROOT
    // =========================================================

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot")
        );

    SetRootComponent(SceneRoot);

    // ATimePortal creates PortalMesh as its root and attaches the installed
    // cable and handheld-device components beneath it. Reconnect that
    // inherited hierarchy after replacing the root for the opening tube.
    PortalMesh->SetupAttachment(SceneRoot);


    // =========================================================
    // TUBE MESH
    // =========================================================

    TubeMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("TubeMesh")
        );

    TubeMesh->SetupAttachment(SceneRoot);


    // =========================================================
    // DOOR PIVOT
    // =========================================================

    DoorPivot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("DoorPivot")
        );

    DoorPivot->SetupAttachment(SceneRoot);


    // =========================================================
    // DOOR MESH
    // =========================================================

    DoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("DoorMesh")
        );

    DoorMesh->SetupAttachment(DoorPivot);
}


// =============================================================
// BEGIN PLAY
// =============================================================

void AOpeningTube::BeginPlay()
{
    Super::BeginPlay();


    if (DoorPivot)
    {
        ClosedDoorRotation =
            DoorPivot->GetRelativeRotation();

        TargetDoorRotation =
            ClosedDoorRotation +
            OpenDoorRotation;
    }


    // This actor is the opening/arrival tube, so the sequence always runs.
    // Saved Blueprint defaults must not suppress it.
    if (true)
    {
        SetPlayerControlEnabled(false);


        GetWorldTimerManager().SetTimer(
            OpeningTimerHandle,
            this,
            &AOpeningTube::StartOpening,
            OpeningDelay,
            false
        );
    }
}


// =============================================================
// TICK
// =============================================================

void AOpeningTube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);


    if (
        (!bDoorIsOpening && !bDoorIsClosing) ||
        bSequenceFinished ||
        !DoorPivot
        )
    {
        return;
    }


    OpeningElapsedTime += DeltaTime;


    const float SafeDuration =
        FMath::Max(
            OpeningDuration,
            KINDA_SMALL_NUMBER
        );


    const float Alpha =
        FMath::Clamp(
            OpeningElapsedTime / SafeDuration,
            0.0f,
            1.0f
        );


    const FRotator CurrentRotation = bDoorIsClosing
        ? FMath::Lerp(TargetDoorRotation, ClosedDoorRotation, Alpha)
        : FMath::Lerp(ClosedDoorRotation, TargetDoorRotation, Alpha);


    DoorPivot->SetRelativeRotation(
        CurrentRotation
    );


    if (Alpha >= 1.0f)
    {
        if (bDoorIsClosing)
        {
            bDoorIsClosing = false;
            DoorPivot->SetRelativeRotation(ClosedDoorRotation);
        }
        else
        {
            FinishOpening();
        }
    }
}


// =============================================================
// START OPENING
// =============================================================

void AOpeningTube::StartOpening()
{
    if (
        bDoorIsOpening ||
        bSequenceFinished
        )
    {
        return;
    }


    bDoorIsOpening = true;
    OpeningElapsedTime = 0.0f;


    if (OpeningSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            OpeningSound,
            GetActorLocation()
        );
    }
}


// =============================================================
// FINISH OPENING
// =============================================================

void AOpeningTube::FinishOpening()
{
    if (bSequenceFinished)
    {
        return;
    }


    bDoorIsOpening = false;
    bSequenceFinished = true;


    if (DoorPivot)
    {
        DoorPivot->SetRelativeRotation(
            TargetDoorRotation
        );
    }


    SetPlayerControlEnabled(true);
}


void AOpeningTube::BeginPortalTravelSequence()
{
    SetPlayerControlEnabled(false);
    bSequenceFinished = false;
    bDoorIsOpening = false;
    bDoorIsClosing = true;
    OpeningElapsedTime = 0.0f;

    if (OpeningSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            OpeningSound,
            GetActorLocation()
        );
    }

    const float SafeFlickerInterval = FMath::Max(
        LightFlickerInterval,
        0.03f
    );
    const float SafeTravelDelay = FMath::Max(
        PortalTravelDelay,
        0.1f
    );

    GetWorldTimerManager().SetTimer(
        LightFlickerTimerHandle,
        this,
        &AOpeningTube::ToggleInteriorLight,
        SafeFlickerInterval,
        true
    );

    GetWorldTimerManager().SetTimer(
        PortalTravelTimerHandle,
        this,
        &AOpeningTube::FinishPortalTravelSequence,
        SafeTravelDelay,
        false
    );
}


void AOpeningTube::ToggleInteriorLight()
{
    if (InteriorPowerLight)
    {
        InteriorPowerLight->SetVisibility(
            !InteriorPowerLight->IsVisible()
        );
    }
}


void AOpeningTube::FinishPortalTravelSequence()
{
    GetWorldTimerManager().ClearTimer(LightFlickerTimerHandle);

    if (InteriorPowerLight)
    {
        InteriorPowerLight->SetVisibility(true);
    }

    CompletePortalTravel();
}


// =============================================================
// PLAYER CONTROL
// =============================================================

void AOpeningTube::SetPlayerControlEnabled(
    bool bEnabled
)
{
    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


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


    if (bEnabled)
    {
        PlayerPawn->EnableInput(
            PlayerController
        );

        PlayerController->SetIgnoreMoveInput(false);
        PlayerController->SetIgnoreLookInput(false);
    }
    else
    {
        PlayerPawn->DisableInput(
            PlayerController
        );

        PlayerController->SetIgnoreMoveInput(true);
        PlayerController->SetIgnoreLookInput(true);
    }
}


void AOpeningTube::ApplyInstalledVisualTransforms()
{
    // Exact relative placements approved in BP_OpeningTube.
    AuthoredCableRelativeTransform = FTransform(
        FRotator::ZeroRotator,
        FVector(87.201498f, 41.814316f, 85.694206f),
        FVector(1.0f));

    // FRotator constructor order is Pitch (Y), Yaw (Z), Roll (X).
    AuthoredHandheldRelativeTransform = FTransform(
        FRotator(-0.464048f, 79.783880f, 90.083629f),
        FVector(-30.871726f, -13.500260f, 7.180146f),
        FVector(0.363806f));
}
