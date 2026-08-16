#include "OpeningTube.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AOpeningTube::AOpeningTube()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot"));

    RootComponent = SceneRoot;

    TubeMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("TubeMesh"));

    TubeMesh->SetupAttachment(SceneRoot);

    DoorPivot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("DoorPivot"));

    DoorPivot->SetupAttachment(SceneRoot);

    DoorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("DoorMesh"));

    DoorMesh->SetupAttachment(DoorPivot);
}

void AOpeningTube::BeginPlay()
{
    Super::BeginPlay();

    ClosedDoorRotation =
        DoorPivot->GetRelativeRotation();

    TargetDoorRotation =
        ClosedDoorRotation + OpenDoorRotation;

    SetPlayerControlEnabled(false);

    GetWorldTimerManager().SetTimer(
        OpeningTimerHandle,
        this,
        &AOpeningTube::StartOpening,
        OpeningDelay,
        false);
}

void AOpeningTube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bDoorIsOpening || bSequenceFinished)
    {
        return;
    }

    OpeningElapsedTime += DeltaTime;

    const float OpeningAlpha =
        FMath::Clamp(
            OpeningElapsedTime / OpeningDuration,
            0.0f,
            1.0f);

    const float SmoothAlpha =
        FMath::InterpEaseInOut(
            0.0f,
            1.0f,
            OpeningAlpha,
            2.0f);

    const FRotator NewRotation =
        FMath::Lerp(
            ClosedDoorRotation,
            TargetDoorRotation,
            SmoothAlpha);

    DoorPivot->SetRelativeRotation(NewRotation);

    if (OpeningAlpha >= 1.0f)
    {
        FinishOpening();
    }
}

void AOpeningTube::StartOpening()
{
    if (OpeningSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            OpeningSound,
            GetActorLocation());
    }

    OpeningElapsedTime = 0.0f;
    bDoorIsOpening = true;
}

void AOpeningTube::FinishOpening()
{
    bDoorIsOpening = false;
    bSequenceFinished = true;

    DoorPivot->SetRelativeRotation(
        TargetDoorRotation);

    SetPlayerControlEnabled(true);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Opening tube sequence finished."));
}

void AOpeningTube::SetPlayerControlEnabled(
    bool bEnabled)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    PlayerController->SetIgnoreMoveInput(!bEnabled);
    PlayerController->SetIgnoreLookInput(!bEnabled);
}