#include "RepairItem.h"
#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ARepairItem::ARepairItem()
{
    PrimaryActorTick.bCanEverTick = false;

    ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
    RootComponent = ItemMesh;

    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

    ItemID = NAME_None;
    bCollected = false;

    static ConstructorHelpers::FObjectFinder<USoundBase> PickupSoundAsset(
        TEXT("/Engine/VREditor/Sounds/UI/Object_PickUp.Object_PickUp"));
    if (PickupSoundAsset.Succeeded())
    {
        PickupSound = PickupSoundAsset.Object;
    }
}

void ARepairItem::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &ARepairItem::CollectItem);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &ARepairItem::CollectItem);

            InputComponent->KeyBindings.Last().bConsumeInput = false;
        }

        DisableInput(PlayerController);
    }

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &ARepairItem::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &ARepairItem::OnPlayerLeaveRange);

    FindGameHUDWidget();
}

void ARepairItem::CollectItem()
{
    if (bCollected)
    {
        return;
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (PlayerController)
    {
        APawn* PlayerPawn = PlayerController->GetPawn();

        if (PlayerPawn)
        {
            UPlayerInventoryComponent* Inventory =
                PlayerPawn->FindComponentByClass<UPlayerInventoryComponent>();

            if (Inventory)
            {
                const FString PickupClassName = GetClass()->GetName();

                if (PickupClassName.Contains(TEXT("CalibrationReader")))
                {
                    Inventory->AddCalibrationReader();
                }
                else if (PickupClassName.Contains(TEXT("PressureCanister")))
                {
                    Inventory->AddPressureCanister();
                }
                else if (PickupClassName.Contains(TEXT("ResonanceConduit")))
                {
                    Inventory->AddResonanceConduit();
                }
            }
            else
            {
                UE_LOG(
                    LogTemp,
                    Warning,
                    TEXT("PlayerInventoryComponent was not found while collecting %s."),
                    *GetName());
            }
        }
    }

    bCollected = true;

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, PickupSound, GetActorLocation());
    }

    FindGameHUDWidget();
    if (GameHUDWidget)
    {
        GameHUDWidget->HideInteractionPrompt();
    }

    ItemMesh->SetVisibility(false);
    ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InteractionTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    DisableInput(PlayerController);
}

void ARepairItem::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController && OtherActor == PlayerController->GetPawn() && !bCollected)
    {
        EnableInput(PlayerController);

        FindGameHUDWidget();
        if (GameHUDWidget)
        {
            GameHUDWidget->ShowInteractionPromptWithText(GetPickupPrompt());
        }
    }
}

void ARepairItem::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

    if (PlayerController && OtherActor == PlayerController->GetPawn())
    {
        DisableInput(PlayerController);

        FindGameHUDWidget();
        if (GameHUDWidget)
        {
            GameHUDWidget->HideInteractionPrompt();
        }
    }
}

void ARepairItem::FindGameHUDWidget()
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

FText ARepairItem::GetPickupPrompt() const
{
    const FString PickupClassName = GetClass()->GetName();

    if (PickupClassName.Contains(TEXT("CalibrationReader")))
    {
        return FText::FromString(TEXT("[E] PICK UP CALIBRATION READER"));
    }
    if (PickupClassName.Contains(TEXT("PressureCanister")))
    {
        return FText::FromString(TEXT("[E] PICK UP PRESSURE CANISTER"));
    }
    if (PickupClassName.Contains(TEXT("ResonanceConduit")))
    {
        return FText::FromString(TEXT("[E] PICK UP RESONANCE CONDUIT"));
    }

    return FText::FromString(TEXT("[E] PICK UP ITEM"));
}

bool ARepairItem::IsCollected() const
{
    return bCollected;
}

FName ARepairItem::GetItemID() const
{
    return ItemID;
}
