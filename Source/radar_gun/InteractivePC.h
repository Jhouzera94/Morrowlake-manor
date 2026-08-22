#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractivePC.generated.h"

class AGenerator;
class UAudioComponent;
class UBoxComponent;
class UGameHUDWidget;
class UMaterialInstanceDynamic;
class USceneComponent;
class USoundBase;
class UStaticMeshComponent;
class UTexture;

USTRUCT(BlueprintType)
struct FPCMonitorTarget
{
    GENERATED_BODY()

    /** Actor which owns the existing monitor mesh. May also be this PC actor. */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Monitor")
    AActor* Actor = nullptr;

    /** Optional component name, e.g. SCREEN, SCREEN1 ... SCREEN6. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitor")
    FName ComponentName = NAME_None;

    /** Material slot containing the visible display surface. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitor", meta = (ClampMin = "0"))
    int32 MaterialIndex = 0;

    /** This monitor's unique image after booting. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitor")
    UTexture* FinalTexture = nullptr;
};

UENUM(BlueprintType)
enum class EPCBootState : uint8
{
    Off,
    Booting,
    On
};

/** Controls an existing PC tower and any number of existing monitor meshes. */
UCLASS(Blueprintable)
class RADAR_GUN_API AInteractivePC : public AActor
{
    GENERATED_BODY()

public:
    AInteractivePC();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC|Interaction")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC|Interaction")
    UBoxComponent* InteractionTrigger;

    /** Existing monitors; normally seven entries named SCREEN through SCREEN6. */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PC|Monitors")
    TArray<FPCMonitorTarget> Monitors;

    /** Rebind the monitor entries to screen actors in this PC's Outliner folder.
     *  A nearest-screen fallback is used outside the editor. This prevents a
     *  duplicated PC controller from continuing to operate the original PC. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Monitors")
    bool bAutoDiscoverNearbyMonitors = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Monitors",
        meta = (EditCondition = "bAutoDiscoverNearbyMonitors", ClampMin = "100.0", UIMin = "100.0"))
    float MonitorDiscoveryRadius = 1200.0f;

    /** Existing mansion generator. If unassigned, the level's AGenerator is found automatically. */
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PC|Power")
    AGenerator* PowerGenerator = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Boot")
    UTexture* BootTexture = nullptr;

    /** Black texture shown while the PC is powered off. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Boot")
    UTexture* PoweredOffTexture = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Boot", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float BootDuration = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Sound")
    USoundBase* PCStartupSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Sound")
    USoundBase* MonitorStartupSound = nullptr;

    /** Half-size of the interaction cube in centimetres. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Interaction", meta = (ClampMin = "25.0", UIMin = "25.0"))
    float InteractionDistance = 150.0f;

    /** Texture parameter expected by the monitor display material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Material")
    FName TextureParameterName = TEXT("ScreenTexture");

    /** Scalar parameter expected by the monitor display material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Material")
    FName EmissiveParameterName = TEXT("EmissiveStrength");

    /** Scalar material parameter multiplied into both Base Color and Emissive. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Material")
    FName ScreenPowerParameterName = TEXT("ScreenPower");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PC|Material", meta = (ClampMin = "0.0"))
    float PoweredOnEmissiveStrength = 8.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PC|State")
    EPCBootState BootState = EPCBootState::Off;

    UFUNCTION(BlueprintCallable, Category = "PC")
    void Interact();

    UFUNCTION(BlueprintPure, Category = "PC")
    bool HasMansionPower() const;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerLeave(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    void FinishBoot();
    void AutoDiscoverMonitorActors();
    void CreateMonitorMaterials();
    void SetAllMonitors(UTexture* Texture, float EmissiveStrength, float ScreenPower);
    void SetFinalMonitorTextures();
    UStaticMeshComponent* ResolveMonitorMesh(const FPCMonitorTarget& Target) const;
    void FindHUD();
    void RefreshPrompt();
    void HidePrompt();

    UPROPERTY(Transient)
    TArray<UMaterialInstanceDynamic*> MonitorMaterials;

    UPROPERTY(Transient)
    UGameHUDWidget* HUD = nullptr;

    bool bPlayerNearby = false;
    FTimerHandle BootTimer;
};
