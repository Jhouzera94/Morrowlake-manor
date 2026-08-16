#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlueprintLocationVolume.generated.h"

class UBoxComponent;

/**
 * Place one volume in each important room. When the player enters it, the
 * collected house blueprint receives the room name and a normalized marker
 * position (0..1 across the evidence widget).
 */
UCLASS()
class RADAR_GUN_API ABlueprintLocationVolume : public AActor
{
    GENERATED_BODY()

public:
    ABlueprintLocationVolume();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    UBoxComponent* LocationVolume;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    FName FloorID = TEXT("GroundFloor");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    FText FloorName = FText::FromString(TEXT("GROUND FLOOR"));

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blueprint Location")
    FText RoomName = FText::FromString(TEXT("UNKNOWN ROOM"));

    /** Position relative to the full blueprint widget: (0,0) top-left, (1,1) bottom-right. */
    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Blueprint Location",
        meta = (ClampMin = "0.0", ClampMax = "1.0"))
    FVector2D NormalizedMapPosition = FVector2D(0.5f, 0.5f);

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnPlayerEnter(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
};
