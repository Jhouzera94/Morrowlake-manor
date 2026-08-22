#include "TimePortal.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"
#include "TowerCodeEntryWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"


// =========================================================
// CONSTRUCTOR
// =========================================================

ATimePortal::ATimePortal()
{
    PrimaryActorTick.bCanEverTick = false;

    PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
    RootComponent = PortalMesh;

    DisplayWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("DisplayWidget"));
    DisplayWidget->SetupAttachment(PortalMesh);

    InteriorPowerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("InteriorPowerLight"));
    InteriorPowerLight->SetupAttachment(PortalMesh);
    InteriorPowerLight->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
    InteriorPowerLight->SetLightColor(FLinearColor(0.08f, 1.0f, 0.24f));
    InteriorPowerLight->SetIntensity(4500.0f);
    InteriorPowerLight->SetAttenuationRadius(550.0f);
    InteriorPowerLight->SetVisibility(false);

    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(PortalMesh);
    InteractionTrigger->SetBoxExtent(FVector(240.0f, 240.0f, 260.0f));
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractionTrigger->SetGenerateOverlapEvents(true);

    InstalledCableMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("InstalledCableMesh"));
    InstalledCableMesh->SetupAttachment(PortalMesh);
    InstalledCableMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InstalledCableMesh->SetRelativeLocation(FVector(70.0f, 0.0f, 60.0f));
    InstalledCableMesh->SetVisibility(false);
    InstalledCableMesh->SetHiddenInGame(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CableMeshAsset(
        TEXT("/Game/Abandoned_MAnsion/tool/Portal/SM_cable_type1_C_2.SM_cable_type1_C_2"));
    if (CableMeshAsset.Succeeded())
    {
        InstalledCableMesh->SetStaticMesh(CableMeshAsset.Object);
    }

    InstalledHandheldDeviceMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("InstalledHandheldDeviceMesh"));
    InstalledHandheldDeviceMesh->SetupAttachment(PortalMesh);
    InstalledHandheldDeviceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InstalledHandheldDeviceMesh->SetRelativeLocation(FVector(85.0f, 0.0f, 135.0f));
    InstalledHandheldDeviceMesh->SetVisibility(false);
    InstalledHandheldDeviceMesh->SetHiddenInGame(true);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> HandheldMeshAsset(
        TEXT("/Game/Abandoned_MAnsion/tool/restore_items/Hand_pc.Hand_pc"));
    if (HandheldMeshAsset.Succeeded())
    {
        InstalledHandheldDeviceMesh->SetStaticMesh(HandheldMeshAsset.Object);
    }
}


// =========================================================
// BEGIN PLAY
// =========================================================

void ATimePortal::BeginPlay()
{
    Super::BeginPlay();

    // Blueprint component overrides have been applied by BeginPlay. Capture
    // them now so installation always uses the exact transforms authored in
    // BP_TimePortal, including location, rotation, and scale.
    if (InstalledCableMesh && InstalledHandheldDeviceMesh)
    {
        AuthoredCableRelativeTransform = InstalledCableMesh->GetRelativeTransform();
        AuthoredHandheldRelativeTransform =
            InstalledHandheldDeviceMesh->GetRelativeTransform();
        bCapturedInstalledVisualTransforms = true;
    }

    if (PortalMesh)
    {
        PortalScreenMaterial =
            PortalMesh->CreateAndSetMaterialInstanceDynamic(0);

        if (PortalScreenMaterial)
        {
            PortalScreenMaterial->SetScalarParameterValue(
                TEXT("Power"),
                0.0f
            );

            PortalScreenMaterial->SetScalarParameterValue(
                TEXT("Restoration"),
                0.0f
            );

            PortalScreenMaterial->SetScalarParameterValue(
                TEXT("Loading"),
                0.0f
            );

            PortalScreenMaterial->SetScalarParameterValue(
                TEXT("Error"),
                0.0f
            );

            // Element 0 begins visually inactive until the tower sequence
            // has been completed and sends power to the portal.
            PortalScreenMaterial->SetScalarParameterValue(TEXT("EmissiveIntensity"), 0.0f);
            PortalScreenMaterial->SetScalarParameterValue(TEXT("PortalActive"), 0.0f);
            PortalScreenMaterial->SetVectorParameterValue(
                TEXT("EmissiveColor"), FLinearColor::Black);
        }
    }

    PortalState = EPortalState::Offline;

    RefreshInstalledVisuals();

    if (InteriorPowerLight)
    {
        InteriorPowerLight->SetVisibility(false);
    }

    if (InteractionTrigger)
    {
        InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
            this, &ATimePortal::OnPortalRangeEntered);
        InteractionTrigger->OnComponentEndOverlap.AddDynamic(
            this, &ATimePortal::OnPortalRangeLeft);
    }

    if (APlayerController* PlayerController = GetWorld()
            ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        EnableInput(PlayerController);
        if (InputComponent)
        {
            InputComponent->BindKey(EKeys::E, IE_Pressed, this,
                &ATimePortal::HandlePortalInteract);
            InputComponent->KeyBindings.Last().bConsumeInput = false;
            InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this,
                &ATimePortal::HandlePortalInteract);
            InputComponent->KeyBindings.Last().bConsumeInput = false;
        }
        DisableInput(PlayerController);
    }

    FindGameHUDWidget();

    BP_UpdateDisplay();
}


// =========================================================
// RECEIVE POWER FROM TOWER
// =========================================================

void ATimePortal::ReceiveTowerPower()
{
    if (bTowerPowerReceived)
    {
        return;
    }

    bTowerPowerReceived = true;

    if (InteriorPowerLight)
    {
        InteriorPowerLight->SetVisibility(true);
    }

    PortalState = EPortalState::AwaitingRepairs;

    if (PortalScreenMaterial)
    {
        PortalScreenMaterial->SetScalarParameterValue(
            TEXT("Power"),
            1.0f
        );
        PortalScreenMaterial->SetScalarParameterValue(TEXT("PortalActive"), 1.0f);
        PortalScreenMaterial->SetScalarParameterValue(TEXT("EmissiveIntensity"), 18.0f);
        PortalScreenMaterial->SetVectorParameterValue(
            TEXT("EmissiveColor"), FLinearColor(0.06f, 1.0f, 0.22f, 1.0f));
    }

    BP_OnTowerPowerReceived();

    BP_OnPortalStateChanged(PortalState);

    BP_UpdateDisplay();

    EvaluatePortalState();
    UpdatePortalObjective();
}


// =========================================================
// SUPPLY BLUEPRINT
// =========================================================

void ATimePortal::SupplyBlueprint()
{
    if (!bTowerPowerReceived || bHasBlueprint)
    {
        return;
    }

    bHasBlueprint = true;

    BP_OnRepairItemInstalled();
    RefreshInstalledVisuals();

    UpdateRestorationProgress();

    EvaluatePortalState();
}


// =========================================================
// SUPPLY CABLE
// =========================================================

void ATimePortal::SupplyCable()
{
    if (!bTowerPowerReceived || bHasCable)
    {
        return;
    }

    bHasCable = true;

    if (InstalledCableMesh)
    {
        InstalledCableMesh->SetHiddenInGame(false, true);
        InstalledCableMesh->SetVisibility(true, true);
    }

    BP_OnRepairItemInstalled();
    RefreshInstalledVisuals();

    UpdateRestorationProgress();

    EvaluatePortalState();
}


// =========================================================
// SUPPLY HANDHELD DEVICE
// =========================================================

void ATimePortal::SupplyHandheldDevice()
{
    if (!bTowerPowerReceived || bHasHandheldDevice)
    {
        return;
    }

    bHasHandheldDevice = true;

    if (InstalledHandheldDeviceMesh)
    {
        InstalledHandheldDeviceMesh->SetHiddenInGame(false, true);
        InstalledHandheldDeviceMesh->SetVisibility(true, true);
    }

    BP_OnRepairItemInstalled();
    RefreshInstalledVisuals();

    UpdateRestorationProgress();

    EvaluatePortalState();
}

bool ATimePortal::TryInstallCableFromPlayer()
{
    if (!bTowerPowerReceived || bHasCable)
    {
        return false;
    }

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UPlayerInventoryComponent* Inventory = Pawn
        ? Pawn->FindComponentByClass<UPlayerInventoryComponent>() : nullptr;
    if (!Inventory || !Inventory->RemoveTowerCable())
    {
        return false;
    }

    SupplyCable();
    UpdatePortalObjective();
    return true;
}

bool ATimePortal::TryInstallHandheldDeviceFromPlayer()
{
    if (!bTowerPowerReceived || bHasHandheldDevice)
    {
        return false;
    }

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UPlayerInventoryComponent* Inventory = Pawn
        ? Pawn->FindComponentByClass<UPlayerInventoryComponent>() : nullptr;
    if (!Inventory || !Inventory->RemoveCalibrationReader())
    {
        return false;
    }

    SupplyHandheldDevice();
    UpdatePortalObjective();
    return true;
}

bool ATimePortal::TryRepairWithWrenchFromPlayer()
{
    if (!bTowerPowerReceived || !bHasCable || !bHasHandheldDevice ||
        bWrenchRepairComplete)
    {
        return false;
    }

    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;
    UPlayerInventoryComponent* Inventory = Pawn
        ? Pawn->FindComponentByClass<UPlayerInventoryComponent>() : nullptr;
    if (!Inventory || !Inventory->HasPortalWrench())
    {
        return false;
    }

    WrenchRepairUseCount = FMath::Clamp(
        WrenchRepairUseCount + 1,
        0,
        FMath::Max(RequiredWrenchUses, 1));

    if (WrenchUseSound)
    {
        UGameplayStatics::PlaySound2D(this, WrenchUseSound, 1.0f, 1.0f);
    }

    bWrenchRepairComplete =
        WrenchRepairUseCount >= FMath::Max(RequiredWrenchUses, 1);
    BP_OnRepairItemInstalled();
    RefreshInstalledVisuals();
    UpdateRestorationProgress();
    EvaluatePortalState();
    UpdatePortalObjective();
    return true;
}

void ATimePortal::RefreshInstalledVisuals()
{
    ApplyInstalledVisualTransforms();

    if (InstalledCableMesh)
    {
        // Preserve the component transform authored in BP_TimePortal.
        // Installing the cable should only reveal it, never detach or reposition it.
        if (bHasCable && bCapturedInstalledVisualTransforms && PortalMesh)
        {
            InstalledCableMesh->AttachToComponent(
                PortalMesh, FAttachmentTransformRules::KeepRelativeTransform);
            InstalledCableMesh->SetRelativeTransform(AuthoredCableRelativeTransform);
        }
        InstalledCableMesh->SetRenderInMainPass(true);
        InstalledCableMesh->SetRenderInDepthPass(true);
        InstalledCableMesh->SetOwnerNoSee(false);
        InstalledCableMesh->SetOnlyOwnerSee(false);
        InstalledCableMesh->SetVisibility(bHasCable, true);
        InstalledCableMesh->SetHiddenInGame(!bHasCable, true);
    }

    if (InstalledHandheldDeviceMesh)
    {
        // Preserve the component transform authored in BP_TimePortal.
        // Installing the device should only reveal it.
        if (bHasHandheldDevice && bCapturedInstalledVisualTransforms && PortalMesh)
        {
            InstalledHandheldDeviceMesh->AttachToComponent(
                PortalMesh, FAttachmentTransformRules::KeepRelativeTransform);
            InstalledHandheldDeviceMesh->SetRelativeTransform(
                AuthoredHandheldRelativeTransform);
        }
        InstalledHandheldDeviceMesh->SetRenderInMainPass(true);
        InstalledHandheldDeviceMesh->SetRenderInDepthPass(true);
        InstalledHandheldDeviceMesh->SetOwnerNoSee(false);
        InstalledHandheldDeviceMesh->SetOnlyOwnerSee(false);
        InstalledHandheldDeviceMesh->SetVisibility(bHasHandheldDevice, true);
        InstalledHandheldDeviceMesh->SetHiddenInGame(!bHasHandheldDevice, true);
    }
}

void ATimePortal::ApplyInstalledVisualTransforms()
{
    // Base portals use the transforms authored on their own Blueprint components.
}


// =========================================================
// UPDATE RESTORATION PROGRESS
// =========================================================

void ATimePortal::UpdateRestorationProgress()
{
    int32 CompletedParts = 0;

    if (bHasCable)
    {
        CompletedParts++;
    }

    if (bHasHandheldDevice)
    {
        CompletedParts++;
    }

    if (bWrenchRepairComplete)
    {
        CompletedParts++;
    }

    RestorationProgress =
        (static_cast<float>(CompletedParts) / 3.0f) * 100.0f;

    BP_UpdateDisplay();
}


// =========================================================
// CHECK PORTAL STATE
// =========================================================

void ATimePortal::EvaluatePortalState()
{
    EPortalState PreviousState = PortalState;

    if (!bTowerPowerReceived)
    {
        PortalState = EPortalState::Offline;
    }
    else if (!bHasCable || !bHasHandheldDevice || !bWrenchRepairComplete)
    {
        PortalState = EPortalState::AwaitingRepairs;
    }
    else
    {
        PortalState = EPortalState::AwaitingCode;
    }

    if (PreviousState != PortalState)
    {
        BP_OnPortalStateChanged(PortalState);
    }

    BP_UpdateDisplay();
}


// =========================================================
// SUBMIT DATE CODE
// =========================================================

bool ATimePortal::SubmitDateCode(const FString& Code)
{
    if (PortalState != EPortalState::AwaitingCode)
    {
        return false;
    }

    EnteredDateCode = Code;

    if (EnteredDateCode == RequiredDateCode)
    {
        if (PortalScreenMaterial)
        {
            PortalScreenMaterial->SetScalarParameterValue(
                TEXT("Error"),
                0.0f
            );
        }

        UnlockPortal();

        return true;
    }

    if (PortalScreenMaterial)
    {
        PortalScreenMaterial->SetScalarParameterValue(
            TEXT("Error"),
            1.0f
        );
    }

    BP_OnIncorrectDateCode();

    BP_UpdateDisplay();

    return false;
}


// =========================================================
// UNLOCK PORTAL
// =========================================================

void ATimePortal::UnlockPortal()
{
    PortalState = EPortalState::Restoring;

    RestorationProgress = 100.0f;

    if (PortalScreenMaterial)
    {
        PortalScreenMaterial->SetScalarParameterValue(
            TEXT("Restoration"),
            1.0f
        );
    }

    BP_OnPortalStateChanged(PortalState);

    BP_UpdateDisplay();


    // Portal is now ready to activate.
    PortalState = EPortalState::Ready;

    BP_OnPortalRestored();

    BP_OnPortalStateChanged(PortalState);

    BP_UpdateDisplay();

    UpdatePortalObjective();
}


// =========================================================
// START PORTAL
// =========================================================

void ATimePortal::StartPortal()
{
    if (PortalState != EPortalState::Ready)
    {
        return;
    }

    PortalState = EPortalState::Loading;

    if (PortalScreenMaterial)
    {
        PortalScreenMaterial->SetScalarParameterValue(
            TEXT("Loading"),
            1.0f
        );
    }

    BP_OnPortalStateChanged(PortalState);

    BP_OnPortalLoading();

    BP_UpdateDisplay();

    BeginPortalTravelSequence();
}


void ATimePortal::BeginPortalTravelSequence()
{
    // Portals without a custom travel sequence retain the original
    // immediate-travel behaviour.
    CompletePortalTravel();
}


// =========================================================
// COMPLETE TRAVEL
// =========================================================

void ATimePortal::CompletePortalTravel()
{
    if (PortalState != EPortalState::Loading)
    {
        return;
    }

    TravelToDestination();
}


// =========================================================
// TRAVEL TO DESTINATION MAP
// =========================================================

void ATimePortal::TravelToDestination()
{
    if (DestinationLevel.IsNone())
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("TimePortal: DestinationLevel has not been set.")
        );

        return;
    }

    UGameplayStatics::OpenLevel(
        this,
        DestinationLevel
    );
}

void ATimePortal::FindGameHUDWidget()
{
    if (GameHUDWidget)
    {
        return;
    }

    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, Widgets, UGameHUDWidget::StaticClass(), false);
    if (!Widgets.IsEmpty())
    {
        GameHUDWidget = Cast<UGameHUDWidget>(Widgets[0]);
    }
}

void ATimePortal::UpdatePortalObjective()
{
    FindGameHUDWidget();
    if (!GameHUDWidget)
    {
        return;
    }

    if (!bTowerPowerReceived)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Restore the Energy Tower")),
            FText::FromString(TEXT("The portal has no power.")));
    }
    else if (!bHasCable)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Portal powered")),
            FText::FromString(TEXT("Install a power cable.")));
    }
    else if (!bHasHandheldDevice)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Portal powered")),
            FText::FromString(TEXT("Install a handheld device.")));
    }
    else if (!bWrenchRepairComplete)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Repair the portal")),
            FText::FromString(FString::Printf(
                TEXT("Use the wrench on the portal (%d/%d)."),
                WrenchRepairUseCount,
                FMath::Max(RequiredWrenchUses, 1))));
    }
    else if (PortalState == EPortalState::Ready ||
             PortalState == EPortalState::Loading)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Date entered")),
            FText::FromString(TEXT("Proceed to the portal.")));
    }
    else
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(TEXT("Portal repaired")),
            FText::FromString(TEXT("Enter destination date on the destination computer.")));
    }
}

void ATimePortal::HandlePortalInteract()
{
    if (!bPlayerNearPortal)
    {
        return;
    }

    FindGameHUDWidget();
    if (!bTowerPowerReceived)
    {
        UpdatePortalObjective();
        return;
    }

    if (!bHasCable)
    {
        if (TryInstallCableFromPlayer())
        {
            if (GameHUDWidget)
            {
                GameHUDWidget->ShowInteractionPromptWithText(
                    FText::FromString(TEXT("PORTAL CABLE CONNECTED")));
            }
        }
        else if (GameHUDWidget)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("POWER CABLE REQUIRED")));
        }
        return;
    }

    if (!bHasHandheldDevice)
    {
        if (TryInstallHandheldDeviceFromPlayer())
        {
            if (GameHUDWidget)
            {
                GameHUDWidget->ShowInteractionPromptWithText(
                    FText::FromString(TEXT("HANDHELD DEVICE CONNECTED")));
            }
        }
        else if (GameHUDWidget)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("HANDHELD DEVICE REQUIRED")));
        }
        return;
    }

    if (!bWrenchRepairComplete)
    {
        if (TryRepairWithWrenchFromPlayer())
        {
            if (GameHUDWidget)
            {
                const FString RepairMessage = bWrenchRepairComplete
                    ? TEXT("PORTAL REPAIRED")
                    : FString::Printf(
                        TEXT("WRENCH REPAIR %d/%d"),
                        WrenchRepairUseCount,
                        FMath::Max(RequiredWrenchUses, 1));
                GameHUDWidget->ShowInteractionPromptWithText(
                    FText::FromString(RepairMessage));
            }
        }
        else if (GameHUDWidget)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("WRENCH REQUIRED")));
        }
        return;
    }

    if (PortalState == EPortalState::AwaitingCode)
    {
        if (GameHUDWidget)
        {
            GameHUDWidget->ShowInteractionPromptWithText(
                FText::FromString(TEXT("SET DESTINATION AT THE COMPUTER")));
        }
        return;
    }

    if (PortalState == EPortalState::Ready)
    {
        StartPortal();
        return;
    }

    if (PortalState == EPortalState::Loading)
    {
        return;
    }
}

void ATimePortal::OpenDestinationEntry()
{
    APlayerController* PC = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;
    if (!PC || PortalState != EPortalState::AwaitingCode)
    {
        return;
    }

    if (DestinationEntryWidget && DestinationEntryWidget->IsInViewport())
    {
        return;
    }

    DestinationEntryWidget = CreateWidget<UTowerCodeEntryWidget>(
        PC, UTowerCodeEntryWidget::StaticClass());
    if (DestinationEntryWidget)
    {
        DestinationEntryWidget->SetPortalReference(this);
        DestinationEntryWidget->AddToViewport(50);
    }
}

void ATimePortal::OnPortalRangeEntered(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || OtherActor != PC->GetPawn())
    {
        return;
    }

    bPlayerNearPortal = true;
    EnableInput(PC);
    UpdatePortalObjective();
    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPromptWithText(
            FText::FromString(TEXT("[E / X] INTERACT WITH PORTAL")));
    }
}

void ATimePortal::OnPortalRangeLeft(
    UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    if (!PC || OtherActor != PC->GetPawn())
    {
        return;
    }

    bPlayerNearPortal = false;
    DisableInput(PC);
    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}


// =========================================================
// TICK
// =========================================================

void ATimePortal::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
