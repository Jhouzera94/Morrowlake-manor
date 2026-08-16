// Fill out your copyright notice in the Description page of Project Settings.

#include "Generator.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Sound/SoundBase.h"

AGenerator::AGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    GeneratorMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("GeneratorMesh"));

    RootComponent = GeneratorMesh;

    GeneratorScreenMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("GeneratorScreenMesh"));

    GeneratorScreenMesh->SetupAttachment(GeneratorMesh);
    GeneratorScreenMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    GeneratorAudio =
        CreateDefaultSubobject<UAudioComponent>(
            TEXT("GeneratorAudio"));

    GeneratorAudio->SetupAttachment(RootComponent);
    GeneratorAudio->bAutoActivate = false;

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);

    InteractionTrigger->SetBoxExtent(
        FVector(150.0f, 150.0f, 150.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void AGenerator::BeginPlay()
{
    Super::BeginPlay();

    bIsRunning = false;

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
                &AGenerator::ToggleGenerator);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &AGenerator::ToggleGenerator);
        }

        DisableInput(PlayerController);
    }

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AGenerator::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AGenerator::OnPlayerLeaveRange);

    FindGameHUDWidget();
    UpdateScreenMaterial();
    UpdatePowerEffects();
}

void AGenerator::OnPlayerEnterRange(
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
        ShowInteractionPrompt();
    }
}

void AGenerator::OnPlayerLeaveRange(
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
        HideInteractionPrompt();
    }
}

void AGenerator::ToggleGenerator()
{
    if (!bPlayerIsNearby)
    {
        return;
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    /*
     * First interaction: install the fuse.
     */
    if (!bFuseInstalled)
    {
        APawn* PlayerPawn =
            PlayerController->GetPawn();

        if (!PlayerPawn)
        {
            return;
        }

        UPlayerInventoryComponent* Inventory =
            PlayerPawn->FindComponentByClass<
            UPlayerInventoryComponent>();

        if (!Inventory ||
            !Inventory->HasGeneratorFuse())
        {
            if (MissingFuseSound)
            {
                UGameplayStatics::PlaySoundAtLocation(
                    this,
                    MissingFuseSound,
                    GetActorLocation());
            }

            FindGameHUDWidget();

            if (GameHUDWidget)
            {
                GameHUDWidget->UpdateObjective(
                    FText::FromString(
                        TEXT("A generator fuse is missing")),
                    FText::FromString(
                        TEXT("Search the electrical panel")));

                GameHUDWidget
                    ->ShowInteractionPromptWithText(
                        FText::FromString(
                            TEXT(
                                "GENERATOR FUSE REQUIRED")));
            }

            UE_LOG(
                LogTemp,
                Warning,
                TEXT(
                    "Generator cannot start: fuse missing."));

            return;
        }

        if (Inventory->RemoveGeneratorFuse())
        {
            InstallFuse();
        }

        return;
    }

    /*
     * After the fuse is installed, E toggles the generator.
     */
    bIsRunning = !bIsRunning;

    if (GeneratorAudio)
    {
        if (bIsRunning)
        {
            GeneratorAudio->Play();
        }
        else
        {
            GeneratorAudio->Stop();
        }
    }

    UpdateScreenMaterial();
    UpdatePowerEffects();
    ShowInteractionPrompt();

    FindGameHUDWidget();

    if (GameHUDWidget && bIsRunning)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(
                TEXT(
                    "Power restored to Morrowlake Manor")),
            FText::FromString(
                TEXT(
                    "Return to the manor and investigate")));
    }

    // If the Phase Regulator blueprint is already owned, its repair flow is
    // now the more specific objective and should replace the generic power
    // restoration message immediately.
    if (APawn* PlayerPawn = PlayerController->GetPawn())
    {
        if (UPlayerInventoryComponent* Inventory =
                PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>())
        {
            if (Inventory->HasPhaseRegulatorBlueprint())
            {
                Inventory->RefreshPhaseRegulatorObjective();
            }
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Generator %s"),
        bIsRunning
        ? TEXT("ON")
        : TEXT("OFF"));
}

void AGenerator::InstallFuse()
{
    bFuseInstalled = true;
    bIsRunning = false;

    if (InstallFuseSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            InstallFuseSound,
            GetActorLocation());
    }

    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->UpdateObjective(
            FText::FromString(
                TEXT("Start the generator")),
            FText::FromString(
                TEXT(
                    "The generator fuse is now installed")));
    }

    ShowInteractionPrompt();

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Generator fuse installed."));
}

void AGenerator::UpdateScreenMaterial()
{
    UMaterialInterface* NewMaterial =
        bIsRunning
        ? OnlineScreenMaterial
        : OfflineScreenMaterial;

    if (GeneratorScreenMesh && NewMaterial)
    {
        GeneratorScreenMesh->SetMaterial(0, NewMaterial);
    }
}

void AGenerator::UpdatePowerEffects()
{
    if (PowerCollection)
    {
        UKismetMaterialLibrary::SetScalarParameterValue(
            this,
            PowerCollection,
            TEXT("GeneratorPower"),
            bIsRunning ? 1.0f : 0.0f);
    }
}

void AGenerator::FindGameHUDWidget()
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
        false);

    if (FoundWidgets.Num() > 0)
    {
        GameHUDWidget =
            Cast<UGameHUDWidget>(
                FoundWidgets[0]);
    }
}

void AGenerator::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (!GameHUDWidget)
    {
        return;
    }

    if (!bFuseInstalled)
    {
        APlayerController* PlayerController =
            GetWorld()->GetFirstPlayerController();

        UPlayerInventoryComponent* Inventory = nullptr;

        if (PlayerController &&
            PlayerController->GetPawn())
        {
            Inventory =
                PlayerController->GetPawn()
                ->FindComponentByClass<
                UPlayerInventoryComponent>();
        }

        if (Inventory &&
            Inventory->HasGeneratorFuse())
        {
            GameHUDWidget
                ->ShowInteractionPromptWithText(
                    FText::FromString(
                        TEXT(
                            "[E] INSTALL GENERATOR FUSE")));
        }
        else
        {
            GameHUDWidget
                ->ShowInteractionPromptWithText(
                    FText::FromString(
                        TEXT(
                            "GENERATOR FUSE REQUIRED")));
        }

        return;
    }

    if (bIsRunning)
    {
        GameHUDWidget
            ->ShowInteractionPromptWithText(
                FText::FromString(
                    TEXT(
                        "[E] SWITCH GENERATOR OFF")));
    }
    else
    {
        GameHUDWidget
            ->ShowInteractionPromptWithText(
                FText::FromString(
                    TEXT(
                        "[E] START GENERATOR")));
    }
}

void AGenerator::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

bool AGenerator::IsRunning() const
{
    return bIsRunning;
}

bool AGenerator::IsFuseInstalled() const
{
    return bFuseInstalled;
}
