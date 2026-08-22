#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestinationTerminal.generated.h"

class ATimePortal;
class UBoxComponent;
class UDestinationYearWidget;
class USceneComponent;
class UStaticMeshComponent;
class UWidgetComponent;

UCLASS()
class RADAR_GUN_API ADestinationTerminal : public AActor
{
    GENERATED_BODY()

public:
    ADestinationTerminal();

    UFUNCTION(BlueprintCallable, Category = "Destination Terminal")
    bool SubmitDestinationYear(const FString& Year);

    UFUNCTION(BlueprintPure, Category = "Destination Terminal")
    FString GetRequiredYear() const { return RequiredYear; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Destination Terminal|Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination Terminal|Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* TerminalMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination Terminal|Components",
        meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* MonitorPlane;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destination Terminal|Components",
        meta = (AllowPrivateAccess = "true"))
    UWidgetComponent* DestinationWidgetComponent;

    UPROPERTY(VisibleAnywhere, Category = "Destination Terminal|Components")
    UBoxComponent* InteractionTrigger;

    UPROPERTY(EditAnywhere, Category = "Destination Terminal")
    ATimePortal* PortalReference = nullptr;

    UPROPERTY(EditAnywhere, Category = "Destination Terminal")
    FString RequiredYear = TEXT("1950");

    UPROPERTY(Transient)
    UDestinationYearWidget* DestinationScreenWidget = nullptr;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerLeaveRange(UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor, UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    void HandleInteract();
    void OpenDestinationWidget();
    void CloseDestinationWidget();
    void FindPortal();

    bool bPlayerNearby = false;
};
