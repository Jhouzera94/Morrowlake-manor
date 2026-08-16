#include "BlueprintLocationVolume.h"

#include "PlayerInventoryComponent.h"

#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

ABlueprintLocationVolume::ABlueprintLocationVolume()
{
    PrimaryActorTick.bCanEverTick = false;

    LocationVolume =
        CreateDefaultSubobject<UBoxComponent>(TEXT("LocationVolume"));
    RootComponent = LocationVolume;
    LocationVolume->SetBoxExtent(FVector(250.0f, 250.0f, 150.0f));
    LocationVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void ABlueprintLocationVolume::BeginPlay()
{
    Super::BeginPlay();

    LocationVolume->OnComponentBeginOverlap.AddDynamic(
        this, &ABlueprintLocationVolume::OnPlayerEnter);
}

void ABlueprintLocationVolume::OnPlayerEnter(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController =
        GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;

    if (!PlayerController || OtherActor != PlayerController->GetPawn())
    {
        return;
    }

    if (UPlayerInventoryComponent* Inventory =
            OtherActor->FindComponentByClass<UPlayerInventoryComponent>())
    {
        Inventory->SetBlueprintLocation(
            FloorID,
            FloorName,
            RoomName,
            NormalizedMapPosition);
    }
}
