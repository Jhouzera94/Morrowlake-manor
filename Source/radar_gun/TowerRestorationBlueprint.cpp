#include "TowerRestorationBlueprint.h"

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

ATowerRestorationBlueprint::ATowerRestorationBlueprint()
{
    PrimaryActorTick.bCanEverTick = false;
    BlueprintMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlueprintMesh"));
    RootComponent = BlueprintMesh;
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void ATowerRestorationBlueprint::BeginPlay()
{
    Super::BeginPlay();
    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &ATowerRestorationBlueprint::OnTriggerBegin);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &ATowerRestorationBlueprint::OnTriggerEnd);
}

void ATowerRestorationBlueprint::OnTriggerBegin(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>() || bCollected) return;
    bPlayerIsNearby = true;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        EnableInput(PC);
        if (InputComponent)
        {
            InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATowerRestorationBlueprint::CollectBlueprint);
            InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &ATowerRestorationBlueprint::CollectBlueprint);
        }
    }
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UGameHUDWidget::StaticClass(), false);
    for (UUserWidget* Widget : Widgets)
        if (UGameHUDWidget* HUD = Cast<UGameHUDWidget>(Widget)) HUD->ShowInteractionPrompt();
}

void ATowerRestorationBlueprint::OnTriggerEnd(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    if (!OtherActor || !OtherActor->IsA<ACharacter>()) return;
    bPlayerIsNearby = false;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController()) DisableInput(PC);
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UGameHUDWidget::StaticClass(), false);
    for (UUserWidget* Widget : Widgets)
        if (UGameHUDWidget* HUD = Cast<UGameHUDWidget>(Widget)) HUD->HideInteractionPrompt();
}

void ATowerRestorationBlueprint::CollectBlueprint()
{
    if (!bPlayerIsNearby || bCollected) return;
    bCollected = true;
    if (PickupSound) UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        DisableInput(PC);
        if (APawn* Pawn = PC->GetPawn())
            if (UPlayerInventoryComponent* Inventory = Pawn->FindComponentByClass<UPlayerInventoryComponent>())
            {
                if (EvidenceWidgetClass) Inventory->TowerRestorationEvidenceWidgetClass = EvidenceWidgetClass;
                Inventory->AddTowerRestorationBlueprint();
                Inventory->OpenTowerRestorationEvidence();
            }
    }
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}
