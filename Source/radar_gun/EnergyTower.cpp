#include "EnergyTower.h"

#include "GameHUDWidget.h"
#include "PhaseRegulatorMachine.h"
#include "PlayerInventoryComponent.h"
#include "TimePortal.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialParameterCollection.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"


AEnergyTower::AEnergyTower()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;


    // =========================================================
    // ROOT
    // =========================================================

    SceneRoot =
        CreateDefaultSubobject<USceneComponent>(
            TEXT("SceneRoot")
        );

    SetRootComponent(SceneRoot);


    // =========================================================
    // TOWER MESH
    // =========================================================

    TowerMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("TowerMesh")
        );

    TowerMesh->SetupAttachment(SceneRoot);


    static ConstructorHelpers::FObjectFinder<UStaticMesh>
        TowerAsset(
            TEXT(
                "/Game/Abandoned_MAnsion/tool/tower/"
                "SM_tower_energy.SM_tower_energy"
            )
        );

    if (TowerAsset.Succeeded())
    {
        TowerMesh->SetStaticMesh(
            TowerAsset.Object
        );
    }


    // =========================================================
    // INSTALLED CABLE
    // =========================================================

    InstalledCableMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("InstalledCableMesh")
        );

    InstalledCableMesh->SetupAttachment(SceneRoot);

    // Keep the Blueprint usable even when its component mesh was not saved.
    // Individual Blueprint instances can still override this default.
    static ConstructorHelpers::FObjectFinder<UStaticMesh>
        CableAsset(
            TEXT(
                "/Game/Abandoned_MAnsion/tool/tower/"
                "SM_cable_component.SM_cable_component"
            )
        );

    if (CableAsset.Succeeded())
    {
        InstalledCableMesh->SetStaticMesh(CableAsset.Object);
    }

    InstalledCableMesh->SetVisibility(false);

    InstalledCableMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision
    );

    InstalledCalibrationReaderMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InstalledCalibrationReaderMesh"));
    InstalledCalibrationReaderMesh->SetupAttachment(SceneRoot);
    InstalledCalibrationReaderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InstalledCalibrationReaderMesh->SetVisibility(false);

    // The glowing floor cable reads this collection's TowerPower value.
    static ConstructorHelpers::FObjectFinder<UMaterialParameterCollection>
        DefaultPowerCollection(
            TEXT("/Game/generator/MPC_Power.MPC_Power")
        );

    if (DefaultPowerCollection.Succeeded())
    {
        PowerCollection = DefaultPowerCollection.Object;
    }

    // FObjectFinder is valid only during construction. BeginPlay uses this
    // saved fallback when material slot 55 has not been assigned in Blueprint.
    static ConstructorHelpers::FObjectFinder<UMaterialInterface>
        EnergyCylinderFallbackAsset(
            TEXT("/Game/Time_machine/Neon.Neon")
        );

    if (EnergyCylinderFallbackAsset.Succeeded())
    {
        EnergyCylinderFallbackMaterial =
            EnergyCylinderFallbackAsset.Object;
    }

    static ConstructorHelpers::FClassFinder<UUserWidget>
        TowerCodeWidgetAsset(TEXT("/Game/Abandoned_MAnsion/UI/Towercode_widget"));
    if (TowerCodeWidgetAsset.Succeeded())
    {
        TowerCodeWidgetClass = TowerCodeWidgetAsset.Class;
    }


    // =========================================================
    // CABLE INSTALL TRIGGER
    // =========================================================

    CableInstallTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("CableInstallTrigger")
        );

    CableInstallTrigger->SetupAttachment(SceneRoot);

    CableInstallTrigger->SetBoxExtent(
        FVector(
            150.0f,
            150.0f,
            150.0f
        )
    );

    CableInstallTrigger->SetCollisionProfileName(
        TEXT("Trigger")
    );

    // Do not rely on a project collision preset: the cable socket must always
    // detect the player character and show its interaction prompt.
    CableInstallTrigger->SetCollisionEnabled(
        ECollisionEnabled::QueryOnly
    );
    CableInstallTrigger->SetCollisionResponseToAllChannels(
        ECR_Ignore
    );
    CableInstallTrigger->SetCollisionResponseToChannel(
        ECC_Pawn,
        ECR_Overlap
    );
    CableInstallTrigger->SetGenerateOverlapEvents(true);


    // =========================================================
    // AUDIO
    // =========================================================

    ChargingAudio =
        CreateDefaultSubobject<UAudioComponent>(
            TEXT("ChargingAudio")
        );

    ChargingAudio->SetupAttachment(SceneRoot);
    ChargingAudio->bAutoActivate = false;
    ChargingAudio->bAllowSpatialization = true;


    StableEnergyAudio =
        CreateDefaultSubobject<UAudioComponent>(
            TEXT("StableEnergyAudio")
        );

    StableEnergyAudio->SetupAttachment(SceneRoot);
    StableEnergyAudio->bAutoActivate = false;
    StableEnergyAudio->bAllowSpatialization = true;
}


// =============================================================
// BEGIN PLAY
// =============================================================

void AEnergyTower::BeginPlay()
{
    Super::BeginPlay();

    // Keep placed levels working even when the Connected Portal instance
    // reference was not assigned in the Energy Tower Details panel.
    if (!ConnectedPortal)
    {
        if (UWorld* World = GetWorld())
        {
            for (TActorIterator<ATimePortal> It(World); It; ++It)
            {
                ConnectedPortal = *It;
                break;
            }
        }
    }


    // ---------------------------------------------------------
    // Dynamic tower screen material
    // ---------------------------------------------------------

    if (
        TowerMesh &&
        TowerMesh->GetNumMaterials() >
        ScreenMaterialElement
        )
    {
        ScreenMaterial =
            TowerMesh
            ->CreateAndSetMaterialInstanceDynamic(
                ScreenMaterialElement
            );
    }

    // Material slot 55 is the vertical energy cylinder inside the tower.
    // Its material uses EnergyFill (0 = bottom empty, 1 = fully filled).
    if (
        TowerMesh &&
        TowerMesh->GetNumMaterials() >
        EnergyCylinderMaterialElement
        )
    {
        // Element 55 was empty in the placed Blueprint.  Give the energy
        // cylinder a visible fallback material before making it dynamic.
        if (!TowerMesh->GetMaterial(EnergyCylinderMaterialElement))
        {
            if (EnergyCylinderFallbackMaterial)
            {
                TowerMesh->SetMaterial(
                    EnergyCylinderMaterialElement,
                    EnergyCylinderFallbackMaterial
                );
            }
        }

        EnergyCylinderMaterial =
            TowerMesh
            ->CreateAndSetMaterialInstanceDynamic(
                EnergyCylinderMaterialElement
            );
    }


    // ---------------------------------------------------------
    // Cable starts hidden unless already installed
    // ---------------------------------------------------------

    if (InstalledCableMesh)
    {
        InstalledCableMesh->SetVisibility(
            bTowerCableInstalled
        );
    }

    if (InstalledCalibrationReaderMesh)
    {
        InstalledCalibrationReaderMesh->SetVisibility(bComponent1Installed);
    }


    // ---------------------------------------------------------
    // Tower cable begins unpowered
    // ---------------------------------------------------------

    if (PowerCollection)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(
            this,
            PowerCollection,
            TEXT("TowerPower"),
            0.0f
        );
    }


    // ---------------------------------------------------------
    // Bind cable interaction
    // ---------------------------------------------------------

    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


    if (PlayerController)
    {
        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &AEnergyTower::HandleCableInteract
            );

            // Other nearby interactables also use E.  Do not let this tower
            // consume the key and prevent the input stack from updating.
            if (InputComponent->KeyBindings.Num() > 0)
            {
                InputComponent->KeyBindings.Last().bConsumeInput = false;
            }


            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &AEnergyTower::HandleCableInteract
            );


            if (InputComponent->KeyBindings.Num() > 0)
            {
                InputComponent
                    ->KeyBindings
                    .Last()
                    .bConsumeInput = false;
            }
        }

        DisableInput(PlayerController);
    }


    if (CableInstallTrigger)
    {
        CableInstallTrigger
            ->OnComponentBeginOverlap
            .AddDynamic(
                this,
                &AEnergyTower::
                OnCableInstallEnterRange
            );


        CableInstallTrigger
            ->OnComponentEndOverlap
            .AddDynamic(
                this,
                &AEnergyTower::
                OnCableInstallLeaveRange
            );
    }


    FindGameHUDWidget();

    ApplyChargeVisuals();
}


// =============================================================
// PLAYER ENTERS TOWER CABLE RANGE
// =============================================================

void AEnergyTower::OnCableInstallEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


    if (
        !PlayerController ||
        OtherActor != PlayerController->GetPawn()
        )
    {
        return;
    }


    bPlayerNearCableInstall = true;

    EnableInput(PlayerController);

    UpdateCableInteractionPrompt();
    UpdateTowerObjective();
}


// =============================================================
// PLAYER LEAVES TOWER CABLE RANGE
// =============================================================

void AEnergyTower::OnCableInstallLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


    if (
        !PlayerController ||
        OtherActor != PlayerController->GetPawn()
        )
    {
        return;
    }


    bPlayerNearCableInstall = false;

    DisableInput(PlayerController);


    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget
            ->HideInteractionPrompt();
    }
}


// =============================================================
// E INTERACTION
// =============================================================

void AEnergyTower::HandleCableInteract()
{
    if (!bPlayerNearCableInstall)
    {
        return;
    }

    if (bTowerCharged && !bAccessCodeAccepted)
    {
        ShowTowerCodeWidget();
        return;
    }

    if (bTowerCableInstalled)
    {
        if (!bComponent1Installed)
        {
            if (TryInstallComponent1FromPlayer())
            {
                FindGameHUDWidget();
                if (GameHUDWidget)
                {
                    GameHUDWidget->ShowInteractionPromptWithText(
                        FText::FromString(TEXT("HANDHELD DEVICE CONNECTED")));
                }
                StartCharging();
                return;
            }

            FindGameHUDWidget();
            if (GameHUDWidget)
            {
                GameHUDWidget->ShowInteractionPromptWithText(
                    FText::FromString(TEXT("HANDHELD DEVICE REQUIRED")));
            }
        }

        UpdateCableInteractionPrompt();

        return;
    }


    const bool bInstalled =
        TryInstallTowerCableFromPlayer();


    if (bInstalled)
    {
        FindGameHUDWidget();

        if (GameHUDWidget)
        {
            GameHUDWidget
                ->ShowInteractionPromptWithText(
                    FText::FromString(
                        TEXT("POWER CABLE CONNECTED")
                    )
                );

            GameHUDWidget->UpdateObjective(
                FText::FromString(TEXT("Energy Tower")),
                FText::FromString(TEXT("Cable connected. Install a handheld device."))
            );
        }


        // Charging waits for the installed handheld device and for the
        // completed Phase Regulator.
        StartCharging();
    }
    else
    {
        UpdateCableInteractionPrompt();
    }
}


// =============================================================
// HUD PROMPT
// =============================================================

void AEnergyTower::UpdateCableInteractionPrompt()
{
    FindGameHUDWidget();

    if (!GameHUDWidget)
    {
        return;
    }


    if (bTowerCableInstalled)
    {
        UpdateTowerObjective();

        GameHUDWidget
            ->ShowInteractionPromptWithText(
                FText::FromString(
                    TEXT("POWER CABLE CONNECTED")
                )
            );

        return;
    }


    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


    UPlayerInventoryComponent* Inventory =
        nullptr;


    if (
        PlayerController &&
        PlayerController->GetPawn()
        )
    {
        Inventory =
            PlayerController
            ->GetPawn()
            ->FindComponentByClass<
            UPlayerInventoryComponent
            >();
    }


    if (
        Inventory &&
        Inventory->HasTowerCable()
        )
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower")),
            FText::FromString(TEXT("Power cable ready. Press E or X to connect it."))
        );

        GameHUDWidget
            ->ShowInteractionPromptWithText(
                FText::FromString(
                    TEXT(
                        "[E / X] CONNECT POWER CABLE"
                    )
                )
            );
    }
    else
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower")),
            FText::FromString(TEXT("A power cable is required to activate this tower."))
        );

        GameHUDWidget
            ->ShowInteractionPromptWithText(
                FText::FromString(
                    TEXT(
                        "POWER CABLE REQUIRED"
                    )
                )
            );

        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Connect the tower")),
            FText::FromString(TEXT("A power cable is required."))
        );
    }
}


// =============================================================
// TOP-LEFT OBJECTIVE
// =============================================================

void AEnergyTower::UpdateTowerObjective()
{
    FindGameHUDWidget();

    if (!GameHUDWidget)
    {
        return;
    }

    if (bTowerCharged && !bAccessCodeAccepted)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower: Access Code Required")),
            FText::FromString(TEXT("Enter the access code on the tower control panel."))
        );
        return;
    }

    if (bTowerCharged)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower Online")),
            FText::FromString(TEXT("Power has been sent to the time machine."))
        );
        return;
    }

    if (bTowerCableInstalled)
    {
        if (!bComponent1Installed)
        {
            GameHUDWidget->UpdateObjective(
                FText::FromString(TEXT("Energy Tower")),
                FText::FromString(TEXT("Cable connected. Install a handheld device."))
            );
            return;
        }

        if (!IsPhaseRegulatorReady())
        {
            GameHUDWidget->UpdateObjective(
                FText::FromString(TEXT("Energy Tower")),
                FText::FromString(TEXT("Phase Regulator must be fully restored before charging."))
            );
            return;
        }

        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower")),
            FText::FromString(TEXT("Cable and handheld device connected. Tower will begin charging."))
        );
        return;
    }

    APlayerController* PlayerController =
        GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    UPlayerInventoryComponent* Inventory =
        PlayerPawn ? PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>() : nullptr;

    GameHUDWidget->UpdateObjective(
        FText::FromString(TEXT("Energy Tower")),
        FText::FromString(
            Inventory && Inventory->HasTowerCable()
            ? TEXT("Connect the power cable. Press E or X while standing at the tower.")
            : TEXT("A power cable is required to activate this tower.")
        )
    );
}


// =============================================================
// FIND HUD
// =============================================================

void AEnergyTower::FindGameHUDWidget()
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
        false
    );


    if (FoundWidgets.Num() > 0)
    {
        GameHUDWidget =
            Cast<UGameHUDWidget>(
                FoundWidgets[0]
            );
    }
}


// =============================================================
// TICK
// =============================================================

void AEnergyTower::Tick(
    float DeltaSeconds
)
{
    Super::Tick(
        DeltaSeconds
    );


    // Fallback proximity check.  Some placed Blueprint instances can have
    // collision overrides, so relying only on the trigger box can leave the
    // player beside the tower without ever receiving an interaction prompt.
    APlayerController* PlayerController =
        GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;

    if (PlayerController && PlayerPawn)
    {
        constexpr float CableInteractionDistance = 350.0f;
        const bool bPlayerShouldBeNearCable =
            FVector::DistSquared(
                PlayerPawn->GetActorLocation(),
                GetActorLocation()
            ) <= FMath::Square(CableInteractionDistance);

        if (bPlayerShouldBeNearCable != bPlayerNearCableInstall)
        {
            bPlayerNearCableInstall = bPlayerShouldBeNearCable;

            if (bPlayerNearCableInstall)
            {
                EnableInput(PlayerController);
                UpdateCableInteractionPrompt();
                UpdateTowerObjective();
            }
            else
            {
                DisableInput(PlayerController);
                FindGameHUDWidget();

                if (GameHUDWidget)
                {
                    GameHUDWidget->HideInteractionPrompt();
                }
            }
        }
    }


    // This also covers completing the Phase Regulator after the cable and
    // handheld device were already installed.
    if (!bIsCharging && !bTowerCharged && bTowerCableInstalled &&
        bComponent1Installed && IsPhaseRegulatorReady())
    {
        StartCharging();
    }

    if (!bIsCharging)
    {
        return;
    }


    ChargeProgress =
        FMath::Clamp(
            ChargeProgress +
            DeltaSeconds /
            FMath::Max(
                ChargingDuration,
                1.0f
            ),
            0.0f,
            1.0f
        );


    if (ChargingAudio)
    {
        ChargingAudio
            ->SetVolumeMultiplier(
                FMath::Lerp(
                    0.55f,
                    1.0f,
                    ChargeProgress
                )
            );


        ChargingAudio
            ->SetPitchMultiplier(
                FMath::Lerp(
                    0.78f,
                    1.22f,
                    ChargeProgress
                )
            );
    }


    ApplyChargeVisuals();


    if (ChargeProgress >= 1.0f)
    {
        FinishCharging();
    }
}


// =============================================================
// COMPONENT 1
// =============================================================

void AEnergyTower::InstallComponent1()
{
    bComponent1Installed = true;
    if (InstalledCalibrationReaderMesh)
    {
        InstalledCalibrationReaderMesh->SetVisibility(true);
    }
    UpdateTowerObjective();
    StartCharging();
}


// =============================================================
// COMPONENT 2
// =============================================================

void AEnergyTower::InstallComponent2()
{
    bComponent2Installed = true;
}


// =============================================================
// COMPONENT 1 PLAYER INSTALL
// =============================================================

bool AEnergyTower::
TryInstallComponent1FromPlayer()
{
    if (bComponent1Installed)
    {
        return false;
    }


    APlayerController* PlayerController = GetWorld()
        ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* PlayerPawn = PlayerController ? PlayerController->GetPawn() : nullptr;
    UPlayerInventoryComponent* Inventory = PlayerPawn
        ? PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>() : nullptr;

    if (!Inventory || !Inventory->RemoveCalibrationReader())
    {
        return false;
    }

    InstallComponent1();

    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower")),
            FText::FromString(TEXT("Handheld device connected. Restore the Phase Regulator to start charging.")));
    }

    return true;
}


// =============================================================
// CABLE PLAYER INSTALL
// =============================================================

bool AEnergyTower::
TryInstallTowerCableFromPlayer()
{
    if (bTowerCableInstalled)
    {
        return false;
    }


    APlayerController* PlayerController =
        GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;


    if (!PlayerController)
    {
        return false;
    }


    APawn* PlayerPawn =
        PlayerController->GetPawn();


    if (!PlayerPawn)
    {
        return false;
    }


    UPlayerInventoryComponent* Inventory =
        PlayerPawn
        ->FindComponentByClass<
        UPlayerInventoryComponent
        >();


    if (!Inventory)
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "EnergyTower: "
                "PlayerInventoryComponent "
                "not found."
            )
        );

        return false;
    }


    if (!Inventory->HasTowerCable())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT(
                "EnergyTower: "
                "Player does not have "
                "Tower Cable."
            )
        );

        return false;
    }


    if (!Inventory->RemoveTowerCable())
    {
        return false;
    }


    bTowerCableInstalled = true;


    if (InstalledCableMesh)
    {
        InstalledCableMesh
            ->SetVisibility(true);
    }


    UE_LOG(
        LogTemp,
        Warning,
        TEXT(
            "EnergyTower: "
            "Tower Cable installed."
        )
    );


    return true;
}


// =============================================================
// ACCESS CODE
// =============================================================

bool AEnergyTower::SubmitAccessCode(
    const FString& Code
)
{
    if (!bTowerCharged || !bComponent1Installed || !bTowerCableInstalled)
    {
        return false;
    }


    bAccessCodeAccepted =
        Code.TrimStartAndEnd()
        .Equals(
            TEXT("energy"),
            ESearchCase::IgnoreCase
        );


    if (bAccessCodeAccepted)
    {
        if (PowerCollection)
        {
            UKismetMaterialLibrary::SetScalarParameterValue(
                this, PowerCollection, TEXT("TowerPower"), 1.0f);
        }
        if (ConnectedPortal)
        {
            ConnectedPortal->ReceiveTowerPower();
        }
        UpdateTowerObjective();
    }

    return bAccessCodeAccepted;
}


// =============================================================
// START CHARGING
// =============================================================

void AEnergyTower::StartCharging()
{
    if (
        bIsCharging ||
        bTowerCharged
        )
    {
        return;
    }


    if (
        !bComponent1Installed ||
        !bTowerCableInstalled ||
        !IsPhaseRegulatorReady()
        )
    {
        return;
    }


    bIsCharging = true;


    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Tower charging...")),
            FText::FromString(TEXT("Wait for the charging cycle to complete."))
        );
    }


    if (
        ChargingAudio &&
        ChargingLoopSound
        )
    {
        ChargingAudio->SetSound(
            ChargingLoopSound
        );

        ChargingAudio->Play();
    }


    ApplyChargeVisuals();
}


// =============================================================
// FINISH CHARGING
// =============================================================

void AEnergyTower::FinishCharging()
{
    bIsCharging = false;

    bTowerCharged = true;

    ChargeProgress = 1.0f;


    if (ChargingAudio)
    {
        ChargingAudio->Stop();
    }


    if (ChargeCompleteSound)
    {
        UGameplayStatics
            ::PlaySoundAtLocation(
                this,
                ChargeCompleteSound,
                GetActorLocation()
            );
    }


    if (
        StableEnergyAudio &&
        StableLoopSound
        )
    {
        StableEnergyAudio
            ->SetSound(
                StableLoopSound
            );

        StableEnergyAudio
            ->Play();
    }


    ApplyChargeVisuals();


    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Energy Tower: Access Code Required")),
            FText::FromString(TEXT("Use the tower control panel to enter the access code."))
        );
    }


    // The portal receives power only after the player enters the correct
    // code in the separate control-panel widget. The player opens that
    // widget manually with E or Xbox X while standing beside the tower.
}

bool AEnergyTower::IsPhaseRegulatorReady() const
{
    if (RequiredPhaseRegulator)
    {
        return RequiredPhaseRegulator->IsMachineOnline();
    }

    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<APhaseRegulatorMachine> It(World); It; ++It)
        {
            return It->IsMachineOnline();
        }
    }

    return false;
}

void AEnergyTower::ShowTowerCodeWidget()
{
    if (ActiveTowerCodeWidget || !TowerCodeWidgetClass)
    {
        return;
    }

    APlayerController* PlayerController = GetWorld()
        ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PlayerController)
    {
        return;
    }

    ActiveTowerCodeWidget = CreateWidget<UUserWidget>(PlayerController, TowerCodeWidgetClass);
    if (!ActiveTowerCodeWidget)
    {
        return;
    }

    ActiveTowerCodeWidget->AddToViewport(50);
    TowerCodeTextBox = Cast<UEditableTextBox>(
        ActiveTowerCodeWidget->GetWidgetFromName(TEXT("code")));
    TowerCodeApplyButton = Cast<UButton>(
        ActiveTowerCodeWidget->GetWidgetFromName(TEXT("apply")));

    // In the designer, "apply" may be the TextBlock label inside the Button
    // rather than the Button itself.  Use that parent button, or the only
    // Button in this small panel, so clicking the visible Apply control works.
    if (!TowerCodeApplyButton)
    {
        if (UWidget* ApplyLabel = ActiveTowerCodeWidget->GetWidgetFromName(TEXT("apply")))
        {
            for (UWidget* Current = ApplyLabel; Current; Current = Current->GetParent())
            {
                if (UButton* ParentButton = Cast<UButton>(Current))
                {
                    TowerCodeApplyButton = ParentButton;
                    break;
                }
            }
        }
    }

    if (!TowerCodeApplyButton && ActiveTowerCodeWidget->WidgetTree)
    {
        TArray<UWidget*> PanelWidgets;
        ActiveTowerCodeWidget->WidgetTree->GetAllWidgets(PanelWidgets);
        for (UWidget* Widget : PanelWidgets)
        {
            if (UButton* CandidateButton = Cast<UButton>(Widget))
            {
                TowerCodeApplyButton = CandidateButton;
                break;
            }
        }
    }

    if (TowerCodeApplyButton)
    {
        // Use one completed-click callback.  Submitting on both press and
        // click can remove the widget while the click is still being handled.
        TowerCodeApplyButton->OnClicked.RemoveDynamic(this, &AEnergyTower::SubmitTowerCodeFromWidget);
        TowerCodeApplyButton->OnClicked.AddDynamic(this, &AEnergyTower::SubmitTowerCodeFromWidget);
    }

    PlayerController->bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    if (TowerCodeTextBox)
    {
        InputMode.SetWidgetToFocus(TowerCodeTextBox->TakeWidget());
        TowerCodeTextBox->SetKeyboardFocus();
    }
    PlayerController->SetInputMode(InputMode);

    // The panel is optional while the player looks for clues.  These close
    // bindings do not submit a code or reset any tower progress.
    EnableInput(PlayerController);
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AEnergyTower::CloseTowerCodePanel);
        InputComponent->KeyBindings.Last().bConsumeInput = false;
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AEnergyTower::CloseTowerCodePanel);
        InputComponent->KeyBindings.Last().bConsumeInput = false;
        InputComponent->BindKey(EKeys::Gamepad_FaceButton_Right, IE_Pressed, this, &AEnergyTower::CloseTowerCodePanel);
        InputComponent->KeyBindings.Last().bConsumeInput = false;
    }
}

void AEnergyTower::CloseTowerCodePanel()
{
    if (ActiveTowerCodeWidget)
    {
        HideTowerCodeWidget();
    }
}

void AEnergyTower::HideTowerCodeWidget()
{
    if (ActiveTowerCodeWidget)
    {
        ActiveTowerCodeWidget->RemoveFromParent();
    }

    ActiveTowerCodeWidget = nullptr;
    TowerCodeTextBox = nullptr;
    TowerCodeApplyButton = nullptr;

    if (APlayerController* PlayerController = GetWorld()
            ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        PlayerController->bShowMouseCursor = false;
        PlayerController->SetInputMode(FInputModeGameOnly());
    }
}

void AEnergyTower::SubmitTowerCodeFromWidget()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            4.0f,
            FColor::Cyan,
            TEXT("Tower: Apply click received")
        );
    }

    if (!TowerCodeTextBox)
    {
        return;
    }

    const bool bAccepted = SubmitAccessCode(TowerCodeTextBox->GetText().ToString());
    UTextBlock* CodeResult = ActiveTowerCodeWidget
        ? Cast<UTextBlock>(ActiveTowerCodeWidget->GetWidgetFromName(TEXT("CodeResult")))
        : nullptr;

    if (bAccepted)
    {
        if (CodeResult)
        {
            CodeResult->SetText(FText::FromString(TEXT("CODE ACCEPTED")));
            CodeResult->SetColorAndOpacity(FSlateColor(FLinearColor(0.1f, 1.0f, 0.25f, 1.0f)));
        }

        // Keep the success result visible briefly, then return the player
        // to normal gameplay after the tower has sent power to the portal.
        if (UWorld* World = GetWorld())
        {
            FTimerHandle ClosePanelTimer;
            World->GetTimerManager().SetTimer(
                ClosePanelTimer,
                this,
                &AEnergyTower::HideTowerCodeWidget,
                0.8f,
                false);
        }
        return;
    }

    if (CodeResult)
    {
        CodeResult->SetText(FText::FromString(TEXT("INCORRECT CODE")));
        CodeResult->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.15f, 0.1f, 1.0f)));
    }

    if (!bAccepted)
    {
        FindGameHUDWidget();
        if (GameHUDWidget)
        {
            GameHUDWidget->UpdateObjective(
                FText::FromString(TEXT("Energy Tower: Access Code Required")),
                FText::FromString(TEXT("Code not accepted. Try again.")));
        }
    }
}


// =============================================================
// RESET
// =============================================================

void AEnergyTower::ResetTower()
{
    HideTowerCodeWidget();

    bIsCharging = false;

    bTowerCharged = false;

    bComponent1Installed = false;

    bComponent2Installed = false;

    bAccessCodeAccepted = false;

    bTowerCableInstalled = false;

    ChargeProgress = 0.0f;


    if (ChargingAudio)
    {
        ChargingAudio->Stop();
    }


    if (StableEnergyAudio)
    {
        StableEnergyAudio->Stop();
    }


    if (InstalledCableMesh)
    {
        InstalledCableMesh
            ->SetVisibility(false);
    }

    if (InstalledCalibrationReaderMesh)
    {
        InstalledCalibrationReaderMesh->SetVisibility(false);
    }


    if (PowerCollection)
    {
        UKismetMaterialLibrary
            ::SetScalarParameterValue(
                this,
                PowerCollection,
                TEXT("TowerPower"),
                0.0f
            );
    }


    ApplyChargeVisuals();
}


// =============================================================
// MATERIAL VISUALS
// =============================================================

void AEnergyTower::ApplyChargeVisuals()
{
    const float EnergyAmount =
        (bIsCharging || bTowerCharged)
        ? ChargeProgress
        : 0.0f;

    const FLinearColor OfflineScreenColor =
        FLinearColor(0.005f, 0.008f, 0.005f, 1.0f);
    const FLinearColor NeonGreen =
        FLinearColor(0.05f, 1.0f, 0.18f, 1.0f);

    if (ScreenMaterial)
    {
        ScreenMaterial
            ->SetScalarParameterValue(
                TEXT("ChargeProgress"),
                ChargeProgress
            );

        ScreenMaterial
            ->SetScalarParameterValue(
                TEXT("EnergyFill"),
                EnergyAmount
            );

        ScreenMaterial
            ->SetScalarParameterValue(
                TEXT("ScreenOnline"),
                (bIsCharging || bTowerCharged) ? 1.0f : 0.0f
            );

        ScreenMaterial
            ->SetScalarParameterValue(
                TEXT("TowerCharged"),
                bTowerCharged ? 1.0f : 0.0f
            );

        ScreenMaterial
            ->SetVectorParameterValue(
                TEXT("ScreenColor"),
                bTowerCharged ? NeonGreen : OfflineScreenColor
            );

        ScreenMaterial
            ->SetVectorParameterValue(
                TEXT("EmissiveColor"),
                bTowerCharged ? NeonGreen : OfflineScreenColor
            );
    }

    if (EnergyCylinderMaterial)
    {
        EnergyCylinderMaterial
            ->SetScalarParameterValue(TEXT("EnergyFill"), EnergyAmount);
        EnergyCylinderMaterial
            ->SetScalarParameterValue(TEXT("FillAmount"), EnergyAmount);
        EnergyCylinderMaterial
            ->SetScalarParameterValue(
                TEXT("GlowIntensity"),
                bTowerCharged ? 18.0f : (bIsCharging ? 8.0f : 0.0f)
            );
        EnergyCylinderMaterial
            ->SetVectorParameterValue(TEXT("EnergyColor"), NeonGreen);
        EnergyCylinderMaterial
            ->SetVectorParameterValue(TEXT("EmissiveColor"), NeonGreen);
    }
}
