#include "PhaseRegulatorMachine.h"

#include "GameHUDWidget.h"
#include "Generator.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

APhaseRegulatorMachine::APhaseRegulatorMachine()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    MachineMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineMesh"));
    MachineMesh->SetupAttachment(SceneRoot);

    ScreenMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScreenMesh"));
    ScreenMesh->SetupAttachment(MachineMesh);
    ScreenMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CalibrationReaderMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CalibrationReaderMesh"));
    CalibrationReaderMesh->SetupAttachment(SceneRoot);

    PressureCanisterMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PressureCanisterMesh"));
    PressureCanisterMesh->SetupAttachment(SceneRoot);

    ResonanceConduitMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResonanceConduitMesh"));
    ResonanceConduitMesh->SetupAttachment(SceneRoot);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(SceneRoot);
    InteractionTrigger->SetBoxExtent(FVector(180.0f, 180.0f, 140.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

    OperatingAudioComponent =
        CreateDefaultSubobject<UAudioComponent>(TEXT("OperatingAudio"));
    OperatingAudioComponent->SetupAttachment(SceneRoot);
    OperatingAudioComponent->bAutoActivate = false;
}

void APhaseRegulatorMachine::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this, &APhaseRegulatorMachine::OnPlayerEnterRange);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this, &APhaseRegulatorMachine::OnPlayerLeaveRange);

    if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
    {
        EnableInput(PlayerController);
        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E, IE_Pressed, this, &APhaseRegulatorMachine::UseMachine);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &APhaseRegulatorMachine::UseMachine);

            InputComponent->KeyBindings.Last().bConsumeInput = false;
        }
        DisableInput(PlayerController);
    }

    UpdatePartVisibility();
    UpdateScreenMaterial();
    FindGameHUDWidget();

    if (OperatingAudioComponent)
    {
        OperatingAudioComponent->SetSound(OperatingLoopSound);
    }

    SetActorTickEnabled(false);
}

void APhaseRegulatorMachine::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bMachineStarting)
    {
        SetActorTickEnabled(false);
        return;
    }

    StartupElapsed = FMath::Min(
        StartupElapsed + DeltaSeconds,
        FMath::Max(StartupDuration, 1.0f));

    UpdateStartupHUD();

    if (StartupElapsed >= FMath::Max(StartupDuration, 1.0f))
    {
        FinishMachineStartup();
    }
}

void APhaseRegulatorMachine::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || OtherActor != PlayerController->GetPawn())
    {
        return;
    }

    bPlayerIsNearby = true;
    EnableInput(PlayerController);
    UpdateScreenMaterial();
    UpdateObjective();
}

void APhaseRegulatorMachine::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || OtherActor != PlayerController->GetPawn())
    {
        return;
    }

    bPlayerIsNearby = false;
    DisableInput(PlayerController);
    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

void APhaseRegulatorMachine::UseMachine()
{
    if (!bPlayerIsNearby || bMachineOnline || bMachineStarting)
    {
        return;
    }

    UPlayerInventoryComponent* Inventory = FindPlayerInventory();
    if (!Inventory)
    {
        return;
    }

    if (!IsManorPowerOn() || !Inventory->HasPhaseRegulatorBlueprint())
    {
        UpdateScreenMaterial();
        UpdateObjective();
        return;
    }

    if (GetInstalledPartCount() == 3)
    {
        StartMachineStartup();
        return;
    }

    bool bInstalledPart = false;

    if (!bCalibrationReaderInstalled && Inventory->RemoveCalibrationReader())
    {
        bCalibrationReaderInstalled = true;
        bInstalledPart = true;
    }
    else if (!bPressureCanisterInstalled && Inventory->RemovePressureCanister())
    {
        bPressureCanisterInstalled = true;
        bInstalledPart = true;
    }
    else if (!bResonanceConduitInstalled && Inventory->RemoveResonanceConduit())
    {
        bResonanceConduitInstalled = true;
        bInstalledPart = true;
    }

    if (bInstalledPart)
    {
        if (PartAttachmentSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, PartAttachmentSound, GetActorLocation());
        }

        UpdatePartVisibility();
        UpdateScreenMaterial();
    }

    UpdateObjective();
}

void APhaseRegulatorMachine::UpdatePartVisibility()
{
    CalibrationReaderMesh->SetVisibility(bCalibrationReaderInstalled);
    PressureCanisterMesh->SetVisibility(bPressureCanisterInstalled);
    ResonanceConduitMesh->SetVisibility(bResonanceConduitInstalled);
}

void APhaseRegulatorMachine::UpdateScreenMaterial()
{
    if (!ScreenMesh)
    {
        return;
    }

    UMaterialInterface* NewMaterial = nullptr;

    if (bMachineOnline)
    {
        NewMaterial = OnlineScreenMaterial;
    }
    else if (bMachineStarting)
    {
        NewMaterial = StartingScreenMaterial
            ? StartingScreenMaterial
            : PartRequiredScreenMaterial;
    }
    else if (!IsManorPowerOn())
    {
        NewMaterial = OfflineScreenMaterial;
    }
    else
    {
        UPlayerInventoryComponent* Inventory = FindPlayerInventory();
        NewMaterial =
            Inventory && Inventory->HasPhaseRegulatorBlueprint()
                ? PartRequiredScreenMaterial
                : BlueprintRequiredScreenMaterial;
    }

    if (NewMaterial)
    {
        ScreenMesh->SetMaterial(0, NewMaterial);
    }
}

void APhaseRegulatorMachine::UpdateObjective()
{
    FindGameHUDWidget();
    if (!GameHUDWidget)
    {
        return;
    }

    UPlayerInventoryComponent* Inventory = FindPlayerInventory();

    if (!IsManorPowerOn())
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Restore power to the manor")),
            FText::FromString(TEXT("The Phase Regulator is offline")));
        if (bPlayerIsNearby)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("POWER REQUIRED")));
        }
        return;
    }

    if (!Inventory || !Inventory->HasPhaseRegulatorBlueprint())
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Find the missing machine blueprint")),
            FText::FromString(TEXT("The machine cannot be repaired without it")));
        if (bPlayerIsNearby)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("BLUEPRINT REQUIRED")));
        }
        return;
    }

    if (bMachineStarting)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Phase Regulator starting...")),
            FText::FromString(TEXT("Wait for the startup sequence to complete")));

        UpdateStartupHUD();

        if (bPlayerIsNearby)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("MACHINE STARTING")));
        }
        return;
    }

    if (bMachineOnline)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Phase 2 complete: machine online")),
            FText::FromString(TEXT("Find the tower blueprint")));
        if (bPlayerIsNearby)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("MACHINE ONLINE")));
        }
        return;
    }

    const int32 InstalledParts = GetInstalledPartCount();

    if (InstalledParts == 3)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Activate the repaired Phase Regulator")),
            FText::FromString(TEXT("Press E to begin the startup sequence")));

        if (bPlayerIsNearby)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("[E] ACTIVATE PHASE REGULATOR")));
        }
        return;
    }

    GameHUDWidget->UpdateObjective(
        FText::FromString(TEXT("Return to the Phase Regulator machine")),
        FText::Format(
            FText::FromString(TEXT("Install the collected components - {0}/3")),
            FText::AsNumber(InstalledParts)));

    const bool bHasInstallablePart =
        (!bCalibrationReaderInstalled && Inventory->HasCalibrationReader()) ||
        (!bPressureCanisterInstalled && Inventory->HasPressureCanister()) ||
        (!bResonanceConduitInstalled && Inventory->HasResonanceConduit());

    if (bPlayerIsNearby)
    {
        GameHUDWidget->ShowInteractionPromptWithText(
            FText::FromString(
                bHasInstallablePart
                    ? TEXT("[E] INSTALL MACHINE PART")
                    : TEXT("MACHINE PART REQUIRED")));
    }
}

void APhaseRegulatorMachine::StartMachineStartup()
{
    if (bMachineOnline || bMachineStarting || GetInstalledPartCount() < 3)
    {
        return;
    }

    bMachineStarting = true;
    StartupElapsed = 0.0f;

    if (OperatingAudioComponent && OperatingLoopSound)
    {
        OperatingAudioComponent->SetSound(OperatingLoopSound);
        OperatingAudioComponent->Play();
    }

    SetActorTickEnabled(true);
    UpdateScreenMaterial();
    UpdateObjective();

    UE_LOG(
        LogTemp,
        Log,
        TEXT("Phase Regulator startup sequence began (%.1f seconds)."),
        StartupDuration);
}

void APhaseRegulatorMachine::FinishMachineStartup()
{
    bMachineStarting = false;
    bMachineOnline = true;
    StartupElapsed = FMath::Max(StartupDuration, 1.0f);
    SetActorTickEnabled(false);

    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->HidePhaseRegulatorStartupProgress();
    }

    UpdateScreenMaterial();
    UpdateObjective();

    UE_LOG(LogTemp, Log, TEXT("Phase Regulator is ONLINE."));
}

void APhaseRegulatorMachine::UpdateStartupHUD()
{
    FindGameHUDWidget();
    if (!GameHUDWidget)
    {
        return;
    }

    const float SafeDuration = FMath::Max(StartupDuration, 1.0f);
    const float Progress = FMath::Clamp(StartupElapsed / SafeDuration, 0.0f, 1.0f);
    const float Remaining = FMath::Max(0.0f, SafeDuration - StartupElapsed);

    GameHUDWidget->ShowPhaseRegulatorStartupProgress(Progress, Remaining);
}

void APhaseRegulatorMachine::FindGameHUDWidget()
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

UPlayerInventoryComponent* APhaseRegulatorMachine::FindPlayerInventory() const
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return nullptr;
    }

    return PlayerController->GetPawn()
        ->FindComponentByClass<UPlayerInventoryComponent>();
}

bool APhaseRegulatorMachine::IsManorPowerOn() const
{
    AGenerator* Generator = Cast<AGenerator>(
        UGameplayStatics::GetActorOfClass(this, AGenerator::StaticClass()));
    return Generator && Generator->IsRunning();
}

bool APhaseRegulatorMachine::IsMachineOnline() const
{
    return bMachineOnline;
}

bool APhaseRegulatorMachine::IsMachineStarting() const
{
    return bMachineStarting;
}

float APhaseRegulatorMachine::GetStartupProgress() const
{
    return FMath::Clamp(
        StartupElapsed / FMath::Max(StartupDuration, 1.0f),
        0.0f,
        1.0f);
}

int32 APhaseRegulatorMachine::GetInstalledPartCount() const
{
    return
        (bCalibrationReaderInstalled ? 1 : 0) +
        (bPressureCanisterInstalled ? 1 : 0) +
        (bResonanceConduitInstalled ? 1 : 0);
}
