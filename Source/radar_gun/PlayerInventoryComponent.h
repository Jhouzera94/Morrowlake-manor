#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInventoryComponent.generated.h"

class UTexture2D;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class RADAR_GUN_API UPlayerInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerInventoryComponent();

    // =====================================================
    // KEYS
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Keys")
    bool AddKey(FName KeyID);

    UFUNCTION(BlueprintPure, Category = "Inventory|Keys")
    bool HasKey(FName KeyID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Keys")
    int32 GetKeyCount() const;


    // =====================================================
    // FLASHLIGHT
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddFlashlight();

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasFlashlight() const;


    // =====================================================
    // EVIDENCE / BLUEPRINTS
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddGroundFloorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasGroundFloorBlueprint() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddFirstFloorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasFirstFloorBlueprint() const;

    void OpenFirstFloorEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddPhaseRegulatorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasPhaseRegulatorBlueprint() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddTowerRestorationBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasTowerRestorationBlueprint() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void OpenEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void CloseEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void OpenTowerRestorationEvidence();


    // =====================================================
    // TOOLS
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
    void AddMaintenanceTool();

    UFUNCTION(BlueprintPure, Category = "Inventory|Tools")
    bool HasMaintenanceTool() const;

    /** A separate wrench used only for the portal repair. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
    void AddPortalWrench();

    UFUNCTION(BlueprintPure, Category = "Inventory|Tools")
    bool HasPortalWrench() const;

    /** Switches the dedicated HUD tool display between the collected tools. */
    UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
    void ToggleActiveToolDisplay();


    // =====================================================
    // EXISTING REPAIR ITEMS
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    void AddCalibrationReader();

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    bool HasCalibrationReader() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    int32 GetCalibrationReaderCount() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    bool RemoveCalibrationReader();


    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    void AddPressureCanister();

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    bool HasPressureCanister() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    bool RemovePressureCanister();


    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    void AddResonanceConduit();

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    bool HasResonanceConduit() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    bool RemoveResonanceConduit();


    // =====================================================
    // TOWER CABLE
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Tower")
    void AddTowerCable();

    UFUNCTION(BlueprintPure, Category = "Inventory|Tower")
    bool HasTowerCable() const;

    /** Number of spare cables currently carried.  This is stackable so a
     * second cable remains available for the portal later in the game. */
    UFUNCTION(BlueprintPure, Category = "Inventory|Tower")
    int32 GetTowerCableCount() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Tower")
    bool RemoveTowerCable();


    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    int32 GetHeldRepairItemCount() const;


    // =====================================================
    // OBJECTIVES
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Objectives")
    void RefreshPhaseRegulatorObjective();


    // =====================================================
    // BLUEPRINT LOCATION
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Blueprint Location")
    void SetBlueprintLocation(
        FName FloorID,
        const FText& FloorName,
        const FText& RoomName,
        FVector2D NormalizedMapPosition
    );

    UFUNCTION(BlueprintPure, Category = "Inventory|Blueprint Location")
    bool HasBlueprintLocation() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Blueprint Location")
    FName GetCurrentBlueprintFloorID() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Blueprint Location")
    FText GetCurrentBlueprintFloorName() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Blueprint Location")
    FText GetCurrentBlueprintRoomName() const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Blueprint Location")
    FVector2D GetCurrentBlueprintMapPosition() const;


    FVector2D GetGroundFloorTrackedMapPosition() const;

    FVector2D GetFirstFloorTrackedMapPosition() const;

    FText GetFirstFloorTrackedRoomName() const;

    bool IsPlayerOnGroundFloor() const;

    bool IsPlayerOnFirstFloor() const;


    // =====================================================
    // GENERATOR COMPONENTS
    // =====================================================

    UFUNCTION(BlueprintCallable, Category = "Inventory|Components")
    void AddGeneratorFuse();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Components")
    bool RemoveGeneratorFuse();

    UFUNCTION(BlueprintPure, Category = "Inventory|Components")
    bool HasGeneratorFuse() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetComponentCount() const;


    // =====================================================
    // WIDGET / BLUEPRINT ASSETS
    // =====================================================

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Inventory|Evidence"
    )
    TSubclassOf<UUserWidget> EvidenceWidgetClass;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Inventory|Evidence"
    )
    TSubclassOf<UUserWidget> PhaseRegulatorEvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Evidence")
    TSubclassOf<UUserWidget> TowerRestorationEvidenceWidgetClass;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Inventory|Evidence"
    )
    TSubclassOf<UUserWidget> FirstFloorEvidenceWidgetClass;

    UPROPERTY(
        EditAnywhere,
        BlueprintReadOnly,
        Category = "Inventory|Evidence"
    )
    TSoftObjectPtr<UTexture2D> FirstFloorBlueprintTexture;


protected:

    virtual void BeginPlay() override;

    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction
    ) override;


private:

    // =====================================================
    // INTERNAL FUNCTIONS
    // =====================================================

    void ForceFirstPersonCamera();

    void EnsureGameplayHUD();

    void UpdateHUDComponentCount();

    void UpdateHUDBlueprintCount();

    void UpdateHUDKeyCount();

    void UpdateHUDRepairItemIcons();

    void UpdateActiveToolHUD();

    void HandleToolToggleTrigger(float TriggerValue);

    bool TryOpenEvidenceIndex(int32 EvidenceIndex);

    bool IsEvidenceIndexCollected(int32 EvidenceIndex) const;


    // =====================================================
    // KEYS
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Keys")
    TSet<FName> CollectedKeyIDs;


    // =====================================================
    // BASIC INVENTORY
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    bool bHasFlashlight = false;


    // =====================================================
    // EVIDENCE
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasGroundFloorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasFirstFloorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasPhaseRegulatorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasTowerRestorationBlueprint = false;


    // =====================================================
    // TOOLS
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Tools")
    bool bHasMaintenanceTool = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Tools")
    bool bHasPortalWrench = false;

    bool bShowWrenchAsActiveTool = false;

    bool bToolToggleTriggerHeld = false;


    // =====================================================
    // REPAIR ITEMS
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    int32 CalibrationReaderCount = 0;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    bool bHasPressureCanister = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    bool bHasResonanceConduit = false;


    // =====================================================
    // TOWER ITEMS
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Tower")
    int32 TowerCableCount = 0;


    // =====================================================
    // GENERATOR
    // =====================================================

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Components")
    bool bHasGeneratorFuse = false;


    // =====================================================
    // BLUEPRINT LOCATION
    // =====================================================

    UPROPERTY(
        VisibleAnywhere,
        Category = "Inventory|Blueprint Location"
    )
    bool bHasCurrentBlueprintLocation = false;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Inventory|Blueprint Location"
    )
    FName CurrentBlueprintFloorID = NAME_None;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Inventory|Blueprint Location"
    )
    FText CurrentBlueprintFloorName;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Inventory|Blueprint Location"
    )
    FText CurrentBlueprintRoomName;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Inventory|Blueprint Location"
    )
    FVector2D CurrentBlueprintMapPosition =
        FVector2D(0.5f, 0.5f);


    // =====================================================
    // MAP TRACKING
    // =====================================================

    FVector2D GroundFloorMapWorldSize =
        FVector2D(18000.0f, 18000.0f);

    FVector2D GroundFloorTrackingMapOrigin =
        FVector2D(0.49f, 0.31f);

    float FirstFloorMinimumPlayerZ = 450.0f;

    FVector GroundFloorTrackingOrigin =
        FVector::ZeroVector;

    bool bHasGroundFloorTrackingOrigin = false;


    // =====================================================
    // CURRENT EVIDENCE UI
    // =====================================================

    UPROPERTY()
    UUserWidget* CurrentEvidenceWidget = nullptr;

    // 0 = ground floor
    // 1 = first floor
    // 2 = Phase Regulator schematic
    int32 NextEvidenceIndex = 0;
};
