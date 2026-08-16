#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "PoweredLight.generated.h"

class AGenerator;

UCLASS()
class RADAR_GUN_API APoweredLight : public AActor
{
    GENERATED_BODY()

public:
    APoweredLight();

    UFUNCTION(BlueprintCallable, Category = "Power")
    void ToggleSwitch();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Light")
    UPointLightComponent* Light;

    UPROPERTY(EditInstanceOnly, Category = "Power")
    AGenerator* PowerGenerator;

    UPROPERTY(EditAnywhere, Category = "Power")
    bool bRequiresSwitch = false;

    UPROPERTY(EditAnywhere, Category = "Power")
    bool bSwitchOn = false;

    void UpdateLight();
};
