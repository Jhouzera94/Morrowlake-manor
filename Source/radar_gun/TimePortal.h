#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimePortal.generated.h"

class UStaticMeshComponent;
class UWidgetComponent;
class UMaterialInstanceDynamic;
class UPointLightComponent;
class UBoxComponent;
class UGameHUDWidget;
class UPrimitiveComponent;
class UTowerCodeEntryWidget;
class USoundBase;

UENUM(BlueprintType)
enum class EPortalState : uint8
{
    Offline         UMETA(DisplayName = "Offline"),
    AwaitingRepairs UMETA(DisplayName = "Awaiting Repairs"),
    AwaitingCode    UMETA(DisplayName = "Awaiting Date Code"),
    Restoring       UMETA(DisplayName = "Restoring"),
    Ready           UMETA(DisplayName = "Ready"),
    Loading         UMETA(DisplayName = "Loading")
};

UCLASS()
class RADAR_GUN_API ATimePortal : public AActor
{
    GENERATED_BODY()

public:
    ATimePortal();

    // =========================================================
    // COMPONENTS
    // =========================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Components")
    UStaticMeshComponent* PortalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Components")
    UWidgetComponent* DisplayWidget;

    /** Green guidance light inside the time-machine tube. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Components")
    UPointLightComponent* InteriorPowerLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Components")
    UBoxComponent* InteractionTrigger;

    /** Cable mesh revealed after the player installs the portal cable. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Installed Visuals")
    UStaticMeshComponent* InstalledCableMesh;

    /** Handheld-device mesh revealed after the player installs the device. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Installed Visuals")
    UStaticMeshComponent* InstalledHandheldDeviceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Installed Visuals")
    FVector InstalledCableVisibleLocation = FVector(100.0f, -45.0f, 75.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Installed Visuals")
    FVector InstalledHandheldVisibleLocation = FVector(100.0f, 45.0f, 125.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Installed Visuals")
    FVector InstalledCableVisibleScale = FVector(0.35f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Installed Visuals")
    FVector InstalledHandheldVisibleScale = FVector(0.60f);


    // =========================================================
    // PORTAL STATE
    // =========================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
    EPortalState PortalState = EPortalState::Offline;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
    bool bTowerPowerReceived = false;


    // =========================================================
    // REPAIR REQUIREMENTS
    // =========================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    bool bHasBlueprint = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    bool bHasCable = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    bool bHasHandheldDevice = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    bool bWrenchRepairComplete = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    int32 WrenchRepairUseCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal|Repair",
        meta = (ClampMin = "1", UIMin = "1"))
    int32 RequiredWrenchUses = 5;

    /** Played at the portal once for every wrench repair action. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal|Repair|Sound")
    USoundBase* WrenchUseSound = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Repair")
    float RestorationProgress = 0.0f;


    // =========================================================
    // DATE / DESTINATION
    // =========================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Date")
    FString RequiredDateCode = TEXT("1950");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal|Date")
    FString EnteredDateCode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Portal|Travel")
    FName DestinationLevel = TEXT("Abandoned_Story_1950");


    // =========================================================
    // SCREEN MATERIAL
    // =========================================================

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Portal|Visual")
    UMaterialInstanceDynamic* PortalScreenMaterial;

    UPROPERTY(Transient)
    UTowerCodeEntryWidget* DestinationEntryWidget = nullptr;


    // =========================================================
    // TOWER POWER
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void ReceiveTowerPower();


    // =========================================================
    // REPAIR FUNCTIONS
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    void SupplyBlueprint();

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    void SupplyCable();

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    void SupplyHandheldDevice();

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    bool TryInstallCableFromPlayer();

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    bool TryInstallHandheldDeviceFromPlayer();

    UFUNCTION(BlueprintCallable, Category = "Portal|Repair")
    bool TryRepairWithWrenchFromPlayer();


    // =========================================================
    // DATE COMPUTER
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Portal|Date")
    bool SubmitDateCode(const FString& Code);


    // =========================================================
    // PORTAL ACTIVATION
    // =========================================================

    UFUNCTION(BlueprintCallable, Category = "Portal")
    virtual void StartPortal();

    UFUNCTION(BlueprintCallable, Category = "Portal")
    void CompletePortalTravel();


    // =========================================================
    // BLUEPRINT EVENTS
    // =========================================================

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnTowerPowerReceived();

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnPortalStateChanged(EPortalState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnRepairItemInstalled();

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnIncorrectDateCode();

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnPortalRestored();

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_OnPortalLoading();

    UFUNCTION(BlueprintImplementableEvent, Category = "Portal|Events")
    void BP_UpdateDisplay();


protected:
    virtual void BeginPlay() override;

    void UpdateRestorationProgress();

    void RefreshInstalledVisuals();

    /** Restores the installed meshes to the placement intended by this portal class. */
    virtual void ApplyInstalledVisualTransforms();

    /** Starts the class-specific effects that lead into level travel. */
    virtual void BeginPortalTravelSequence();

    void EvaluatePortalState();

    void UnlockPortal();

    void TravelToDestination();

    void FindGameHUDWidget();
    void UpdatePortalObjective();
    void HandlePortalInteract();
    void OpenDestinationEntry();

    UFUNCTION()
    void OnPortalRangeEntered(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPortalRangeLeft(
        UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    UPROPERTY()
    UGameHUDWidget* GameHUDWidget = nullptr;

    bool bPlayerNearPortal = false;

    /** Relative transforms authored on the inherited components in BP_TimePortal. */
    FTransform AuthoredCableRelativeTransform;
    FTransform AuthoredHandheldRelativeTransform;
    bool bCapturedInstalledVisualTransforms = false;


public:
    virtual void Tick(float DeltaTime) override;
};
