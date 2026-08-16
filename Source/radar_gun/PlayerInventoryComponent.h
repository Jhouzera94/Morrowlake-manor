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

    UFUNCTION(BlueprintCallable, Category = "Inventory|Keys")
    bool AddKey(FName KeyID);

    UFUNCTION(BlueprintPure, Category = "Inventory|Keys")
    bool HasKey(FName KeyID) const;

    UFUNCTION(BlueprintPure, Category = "Inventory|Keys")
    int32 GetKeyCount() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddFlashlight();

    UFUNCTION(BlueprintPure, Category = "Inventory")
    bool HasFlashlight() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddGroundFloorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasGroundFloorBlueprint() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddFirstFloorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasFirstFloorBlueprint() const;

    /** Opens the newly collected first-floor map immediately. */
    void OpenFirstFloorEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void AddPhaseRegulatorBlueprint();

    UFUNCTION(BlueprintPure, Category = "Inventory|Evidence")
    bool HasPhaseRegulatorBlueprint() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void OpenEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Evidence")
    void CloseEvidence();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Tools")
    void AddMaintenanceTool();

    UFUNCTION(BlueprintPure, Category = "Inventory|Tools")
    bool HasMaintenanceTool() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Repair Items")
    void AddCalibrationReader();

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    bool HasCalibrationReader() const;

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

    UFUNCTION(BlueprintPure, Category = "Inventory|Repair Items")
    int32 GetHeldRepairItemCount() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Objectives")
    void RefreshPhaseRegulatorObjective();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Blueprint Location")
    void SetBlueprintLocation(
        FName FloorID,
        const FText& FloorName,
        const FText& RoomName,
        FVector2D NormalizedMapPosition);

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

    /** Ground-floor position kept independent from first-floor room volumes. */
    FVector2D GetGroundFloorTrackedMapPosition() const;

    FVector2D GetFirstFloorTrackedMapPosition() const;
    FText GetFirstFloorTrackedRoomName() const;

    bool IsPlayerOnGroundFloor() const;
    bool IsPlayerOnFirstFloor() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory|Components")
    void AddGeneratorFuse();

    UFUNCTION(BlueprintCallable, Category = "Inventory|Components")
    bool RemoveGeneratorFuse();

    UFUNCTION(BlueprintPure, Category = "Inventory|Components")
    bool HasGeneratorFuse() const;

    UFUNCTION(BlueprintPure, Category = "Inventory")
    int32 GetComponentCount() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Evidence")
    TSubclassOf<UUserWidget> EvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Evidence")
    TSubclassOf<UUserWidget> PhaseRegulatorEvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Evidence")
    TSubclassOf<UUserWidget> FirstFloorEvidenceWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Evidence")
    TSoftObjectPtr<UTexture2D> FirstFloorBlueprintTexture;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    /** Keeps the player in first person even if an old Blueprint input event
        tries to reactivate the third-person follow camera. */
    void ForceFirstPersonCamera();

    /** Creates the gameplay HUD when a map-specific game mode does not. */
    void EnsureGameplayHUD();

    void UpdateHUDComponentCount();
    void UpdateHUDBlueprintCount();
    void UpdateHUDKeyCount();
    void UpdateHUDRepairItemIcons();
    bool TryOpenEvidenceIndex(int32 EvidenceIndex);
    bool IsEvidenceIndexCollected(int32 EvidenceIndex) const;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Keys")
    TSet<FName> CollectedKeyIDs;

    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    bool bHasFlashlight = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasGroundFloorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasFirstFloorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Evidence")
    bool bHasPhaseRegulatorBlueprint = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Tools")
    bool bHasMaintenanceTool = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    bool bHasCalibrationReader = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    bool bHasPressureCanister = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Repair Items")
    bool bHasResonanceConduit = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Components")
    bool bHasGeneratorFuse = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Blueprint Location")
    bool bHasCurrentBlueprintLocation = false;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Blueprint Location")
    FName CurrentBlueprintFloorID = NAME_None;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Blueprint Location")
    FText CurrentBlueprintFloorName;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Blueprint Location")
    FText CurrentBlueprintRoomName;

    UPROPERTY(VisibleAnywhere, Category = "Inventory|Blueprint Location")
    FVector2D CurrentBlueprintMapPosition = FVector2D(0.5f, 0.5f);

    /** Runtime-only tracking state; kept out of reflection so Live Coding
        does not invalidate Blueprint component instances. */
    FVector2D GroundFloorMapWorldSize = FVector2D(18000.0f, 18000.0f);

    // The ground-floor blueprint pickup is in the centre of the lab.
    FVector2D GroundFloorTrackingMapOrigin = FVector2D(0.49f, 0.31f);

    /** Midpoint between the captured ground floor and first floor. */
    float FirstFloorMinimumPlayerZ = 450.0f;

    FVector GroundFloorTrackingOrigin = FVector::ZeroVector;

    bool bHasGroundFloorTrackingOrigin = false;

    UPROPERTY()
    UUserWidget* CurrentEvidenceWidget = nullptr;

    // 0 = ground floor, 1 = first floor, 2 = Phase Regulator schematic.
    int32 NextEvidenceIndex = 0;
};
