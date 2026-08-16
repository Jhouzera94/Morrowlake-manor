#include "KeyPickup.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

AKeyPickup::AKeyPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    KeyMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("KeyMesh"));

    RootComponent = KeyMesh;

    KeyMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);

    InteractionTrigger->SetBoxExtent(
        FVector(80.0f, 80.0f, 80.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void AKeyPickup::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AKeyPickup::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AKeyPickup::OnPlayerLeaveRange);

    FindGameHUD();
}

void AKeyPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bRotateKey && !bCollected)
    {
        AddActorLocalRotation(
            FRotator(
                0.0f,
                RotationSpeed * DeltaTime,
                0.0f));
    }
}

void AKeyPickup::FindGameHUD()
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

void AKeyPickup::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!bCollected &&
        PlayerController &&
        OtherActor == PlayerController->GetPawn())
    {
        bPlayerIsNearby = true;

        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &AKeyPickup::CollectKey);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &AKeyPickup::CollectKey);
        }

        if (!GameHUD)
        {
            FindGameHUD();
        }

        if (GameHUD)
        {
            const FText Prompt =
                FText::Format(
                    FText::FromString(
                        TEXT("[E] TAKE {0}")),
                    KeyDisplayName);

            GameHUD->ShowInteractionPromptWithText(
                Prompt);
        }
    }
}

void AKeyPickup::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!bCollected &&
        PlayerController &&
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

void AKeyPickup::CollectKey()
{
    if (!bPlayerIsNearby || bCollected)
    {
        return;
    }

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

    UPlayerInventoryComponent* Inventory =
        PlayerPawn->FindComponentByClass<
        UPlayerInventoryComponent>();

    if (!Inventory)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PlayerInventoryComponent not found."));

        return;
    }

    if (!Inventory->AddKey(KeyID))
    {
        return;
    }

    bCollected = true;

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation());
    }

    if (GameHUD)
    {
        GameHUD->HideInteractionPrompt();
    }

    DisableInput(PlayerController);
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("%s collected."),
        *KeyDisplayName.ToString());
}