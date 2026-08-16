// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundFloorBlueprint.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

AGroundFloorBlueprint::AGroundFloorBlueprint()
{
    PrimaryActorTick.bCanEverTick = false;

    BlueprintMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("BlueprintMesh"));

    RootComponent = BlueprintMesh;

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);

    InteractionTrigger->SetBoxExtent(
        FVector(120.0f, 120.0f, 100.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void AGroundFloorBlueprint::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AGroundFloorBlueprint::OnTriggerBegin);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AGroundFloorBlueprint::OnTriggerEnd);

    FindGameHUDWidget();
}

void AGroundFloorBlueprint::FindGameHUDWidget()
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
            Cast<UGameHUDWidget>(FoundWidgets[0]);
    }
}

void AGroundFloorBlueprint::ShowInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->ShowInteractionPrompt();
    }
}

void AGroundFloorBlueprint::HideInteractionPrompt()
{
    FindGameHUDWidget();

    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }
}

void AGroundFloorBlueprint::OnTriggerBegin(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    if (!OtherActor ||
        !OtherActor->IsA<ACharacter>() ||
        bCollected)
    {
        return;
    }

    bPlayerIsNearby = true;
    ShowInteractionPrompt();

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    EnableInput(PlayerController);

    if (InputComponent)
    {
        InputComponent->BindKey(
            EKeys::E,
            IE_Pressed,
            this,
            &AGroundFloorBlueprint::CollectBlueprint);

        InputComponent->BindKey(
            EKeys::Gamepad_FaceButton_Left,
            IE_Pressed,
            this,
            &AGroundFloorBlueprint::CollectBlueprint);
    }
}

void AGroundFloorBlueprint::OnTriggerEnd(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    if (!OtherActor ||
        !OtherActor->IsA<ACharacter>())
    {
        return;
    }

    bPlayerIsNearby = false;
    HideInteractionPrompt();

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        DisableInput(PlayerController);
    }
}

void AGroundFloorBlueprint::CollectBlueprint()
{
    if (!bPlayerIsNearby || bCollected)
    {
        return;
    }

    bCollected = true;
    bPlayerIsNearby = false;

    HideInteractionPrompt();

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation());
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        DisableInput(PlayerController);

        APawn* PlayerPawn =
            PlayerController->GetPawn();

        if (PlayerPawn)
        {
            UPlayerInventoryComponent* Inventory =
                PlayerPawn->FindComponentByClass<
                UPlayerInventoryComponent>();

            if (Inventory)
            {
                if (BlueprintKind == EHouseFloorBlueprintKind::FirstFloor)
                {
                    if (EvidenceWidgetClass)
                    {
                        Inventory->FirstFloorEvidenceWidgetClass =
                            EvidenceWidgetClass;
                    }

                    Inventory->AddFirstFloorBlueprint();
                    Inventory->OpenFirstFloorEvidence();
                }
                else
                {
                    if (EvidenceWidgetClass)
                    {
                        Inventory->EvidenceWidgetClass =
                            EvidenceWidgetClass;
                    }

                    Inventory->AddGroundFloorBlueprint();
                    Inventory->OpenEvidence();
                }
            }
        }
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s Blueprint collected."),
        BlueprintKind == EHouseFloorBlueprintKind::FirstFloor
            ? TEXT("First Floor")
            : TEXT("Ground Floor"));

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("New objective: %s"),
        *NextObjective.ToString());

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}
