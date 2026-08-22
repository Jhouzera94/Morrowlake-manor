#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SequentialDialogueTrigger.generated.h"

class UAudioComponent;
class UBoxComponent;
class USoundBase;

/** Independent environmental self-dialogue volume; it never controls the PC. */
UCLASS(Blueprintable)
class RADAR_GUN_API ASequentialDialogueTrigger : public AActor
{
    GENERATED_BODY()

public:
    ASequentialDialogueTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UBoxComponent* TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
    UAudioComponent* DialogueAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<USoundBase*> DialogueLines;

    /** Loop to line one after the final line; otherwise the trigger becomes silent. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bLoopAfterFinalLine = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue|State")
    bool bDialoguePlaying = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue|State")
    int32 NextLineIndex = 0;

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnDialogueFinished();
};
