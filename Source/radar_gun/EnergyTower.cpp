#include "EnergyTower.h"

#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

AEnergyTower::AEnergyTower()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    TowerMesh->SetupAttachment(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> TowerAsset(
        TEXT("/Game/Abandoned_MAnsion/tool/tower/SM_tower_energy.SM_tower_energy")
    );

    if (TowerAsset.Succeeded())
    {
        TowerMesh->SetStaticMesh(TowerAsset.Object);
    }

    ChargingAudio =
        CreateDefaultSubobject<UAudioComponent>(TEXT("ChargingAudio"));

    ChargingAudio->SetupAttachment(SceneRoot);
    ChargingAudio->bAutoActivate = false;
    ChargingAudio->bAllowSpatialization = true;

    StableEnergyAudio =
        CreateDefaultSubobject<UAudioComponent>(TEXT("StableEnergyAudio"));

    StableEnergyAudio->SetupAttachment(SceneRoot);
    StableEnergyAudio->bAutoActivate = false;
    StableEnergyAudio->bAllowSpatialization = true;
}

void AEnergyTower::BeginPlay()
{
    Super::BeginPlay();

    if (TowerMesh &&
        TowerMesh->GetNumMaterials() > ScreenMaterialElement)
    {
        ScreenMaterial =
            TowerMesh->CreateAndSetMaterialInstanceDynamic(
                ScreenMaterialElement
            );
    }

    ApplyChargeVisuals();
}

void AEnergyTower::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!bIsCharging)
    {
        return;
    }

    ChargeProgress = FMath::Clamp(
        ChargeProgress +
        DeltaSeconds / FMath::Max(ChargingDuration, 1.0f),
        0.0f,
        1.0f
    );

    // Charging hum rises in volume and pitch.
    if (ChargingAudio)
    {
        ChargingAudio->SetVolumeMultiplier(
            FMath::Lerp(0.55f, 1.0f, ChargeProgress)
        );

        ChargingAudio->SetPitchMultiplier(
            FMath::Lerp(0.78f, 1.22f, ChargeProgress)
        );
    }

    ApplyChargeVisuals();

    if (ChargeProgress >= 1.0f)
    {
        FinishCharging();
    }
}

void AEnergyTower::InstallComponent1()
{
    bComponent1Installed = true;
}

void AEnergyTower::InstallComponent2()
{
    bComponent2Installed = true;
}

// Attempts to install component 1.
// Returns false if it has already been installed.
bool AEnergyTower::TryInstallComponent1FromPlayer()
{
    if (bComponent1Installed)
    {
        return false;
    }

    InstallComponent1();
    return true;
}

bool AEnergyTower::SubmitAccessCode(const FString& Code)
{
    // Both physical components must be installed first.
    if (!bComponent1Installed || !bComponent2Installed)
    {
        return false;
    }

    // "energy" is case-insensitive.
    bAccessCodeAccepted =
        Code.TrimStartAndEnd().Equals(
            TEXT("energy"),
            ESearchCase::IgnoreCase
        );

    if (bAccessCodeAccepted)
    {
        StartCharging();
    }

    return bAccessCodeAccepted;
}

void AEnergyTower::StartCharging()
{
    if (bIsCharging || bTowerCharged)
    {
        return;
    }

    // Safety check:
    // the puzzle must actually be completed.
    if (!bComponent1Installed ||
        !bComponent2Installed ||
        !bAccessCodeAccepted)
    {
        return;
    }

    bIsCharging = true;

    if (ChargingAudio && ChargingLoopSound)
    {
        ChargingAudio->SetSound(ChargingLoopSound);
        ChargingAudio->Play();
    }

    ApplyChargeVisuals();
}

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
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ChargeCompleteSound,
            GetActorLocation()
        );
    }

    // Permanent/steady powered tower hum.
    if (StableEnergyAudio && StableLoopSound)
    {
        StableEnergyAudio->SetSound(StableLoopSound);
        StableEnergyAudio->Play();
    }

    ApplyChargeVisuals();
}

void AEnergyTower::ResetTower()
{
    bIsCharging = false;
    bTowerCharged = false;
    bComponent1Installed = false;
    bComponent2Installed = false;
    bAccessCodeAccepted = false;

    ChargeProgress = 0.0f;

    if (ChargingAudio)
    {
        ChargingAudio->Stop();
    }

    if (StableEnergyAudio)
    {
        StableEnergyAudio->Stop();
    }

    ApplyChargeVisuals();
}

void AEnergyTower::ApplyChargeVisuals()
{
    if (!ScreenMaterial)
    {
        return;
    }

    ScreenMaterial->SetScalarParameterValue(
        TEXT("ChargeProgress"),
        ChargeProgress
    );

    ScreenMaterial->SetScalarParameterValue(
        TEXT("EnergyFill"),
        ChargeProgress
    );

    ScreenMaterial->SetScalarParameterValue(
        TEXT("ScreenOnline"),
        (bIsCharging || bTowerCharged) ? 1.0f : 0.0f
    );

    ScreenMaterial->SetScalarParameterValue(
        TEXT("TowerCharged"),
        bTowerCharged ? 1.0f : 0.0f
    );
}