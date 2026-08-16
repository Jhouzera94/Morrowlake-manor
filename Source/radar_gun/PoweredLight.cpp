#include "PoweredLight.h"
#include "Generator.h"

APoweredLight::APoweredLight()
{
    PrimaryActorTick.bCanEverTick = true;

    Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
    RootComponent = Light;

    Light->SetVisibility(false);
    Light->SetIntensity(5000.0f);
}

void APoweredLight::BeginPlay()
{
    Super::BeginPlay();
    UpdateLight();
}

void APoweredLight::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateLight();
}

void APoweredLight::ToggleSwitch()
{
    if (bRequiresSwitch)
    {
        bSwitchOn = !bSwitchOn;
        UpdateLight();
    }
}

void APoweredLight::UpdateLight()
{
    const bool bGeneratorOn = PowerGenerator && PowerGenerator->IsRunning();
    const bool bLightShouldBeOn =
        bGeneratorOn && (!bRequiresSwitch || bSwitchOn);

    Light->SetVisibility(bLightShouldBeOn);
}