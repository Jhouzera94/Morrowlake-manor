#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "LightSwitch.generated.h"

class APoweredLight;
class UGameHUDWidget;
class USoundBase;

UCLASS()
class RADAR_GUN_API ALightSwitch : public AActor
{
    GENERATED_BODY()

public:
    ALightSwitch();

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Switch")
    UStaticMeshComponent* SwitchMesh;

    UPROPERTY(VisibleAnywhere, Category = "Switch")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditInstanceOnly, Category = "Switch")
    TArray<APoweredLight*> ControlledLights;

    UPROPERTY(EditAnywhere, Category = "Switch|Sound")
    USoundBase* SwitchOnSound = nullptr;

    UPROPERTY(EditAnywhere, Category = "Switch|Sound")
    USoundBase* SwitchOffSound = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Switch")
    bool bSwitchOn = false;

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    UFUNCTION()
    void UseSwitch();
    void FindGameHUDWidget();
    void ShowInteractionPrompt();
    void HideInteractionPrompt();

    UFUNCTION()
    void OnPlayerEnterRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerLeaveRange(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);
};
