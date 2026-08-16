#include "PlayerInventoryComponent.h"

#include "Generator.h"
#include "GameHUDWidget.h"
#include "EvidenceWidget.h"
#include "PhaseRegulatorMachine.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UPlayerInventoryComponent::UPlayerInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    FirstFloorBlueprintTexture = TSoftObjectPtr<UTexture2D>(
        FSoftObjectPath(
            TEXT("/Game/Abandoned_MAnsion/plantas_casa/Blueprint_First_floor.Blueprint_First_floor")));
}

void UPlayerInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // Abandoned_Story currently contains a PlayerStart below its entrance
    // floor. Recover safely at runtime so both PIE and packaged builds start
    // inside the mansion even if the map asset cannot be checked out.
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        const UWorld* World = GetWorld();
        if (World && World->GetMapName().Contains(TEXT("Abandoned_Story")) &&
            OwnerPawn->GetActorLocation().Z < 400.0)
        {
            FVector SafeLocation = OwnerPawn->GetActorLocation();
            SafeLocation.Z = 540.0;
            OwnerPawn->SetActorLocation(
                SafeLocation, false, nullptr, ETeleportType::TeleportPhysics);
        }
    }

    ForceFirstPersonCamera();
    EnsureGameplayHUD();

    // Keep evidence access independent of the character Blueprint's cached
    // component reference. That reference can be cleared by reinstancing or
    // by changes to the character's input state while playing.
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimerForNextTick(
            FTimerDelegate::CreateWeakLambda(this, [this]()
        {
            EnsureGameplayHUD();
            APawn* OwnerPawn = Cast<APawn>(GetOwner());
            if (OwnerPawn && OwnerPawn->InputComponent)
            {
                OwnerPawn->InputComponent->BindKey(
                    EKeys::Tab,
                    IE_Pressed,
                    this,
                    &UPlayerInventoryComponent::OpenEvidence);

                OwnerPawn->InputComponent->BindKey(
                    EKeys::Gamepad_FaceButton_Top,
                    IE_Pressed,
                    this,
                    &UPlayerInventoryComponent::OpenEvidence);

                OwnerPawn->InputComponent->BindKey(
                    EKeys::Gamepad_FaceButton_Right,
                    IE_Pressed,
                    this,
                    &UPlayerInventoryComponent::CloseEvidence);
            }
        }));
    }

    UpdateHUDComponentCount();
    UpdateHUDBlueprintCount();
    UpdateHUDRepairItemIcons();
}

void UPlayerInventoryComponent::EnsureGameplayHUD()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    APlayerController* PlayerController =
        OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
    if (!PlayerController || !PlayerController->IsLocalController())
    {
        return;
    }

    TArray<UUserWidget*> ExistingHUDs;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, ExistingHUDs, UGameHUDWidget::StaticClass(), false);
    if (!ExistingHUDs.IsEmpty())
    {
        return;
    }

    static TSubclassOf<UUserWidget> HUDClass = LoadClass<UUserWidget>(
        nullptr,
        TEXT("/Game/Abandoned_MAnsion/UI/WBP_HUD.WBP_HUD_C"));
    if (HUDClass)
    {
        if (UUserWidget* HUD = CreateWidget<UUserWidget>(PlayerController, HUDClass))
        {
            HUD->AddToViewport();
        }
    }
}

void UPlayerInventoryComponent::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    ForceFirstPersonCamera();
}

void UPlayerInventoryComponent::ForceFirstPersonCamera()
{
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    TInlineComponentArray<UCameraComponent*> Cameras;
    OwnerActor->GetComponents(Cameras);

    UCameraComponent* FirstPersonCamera = nullptr;
    for (UCameraComponent* Camera : Cameras)
    {
        if (!Camera)
        {
            continue;
        }

        const FString CameraName = Camera->GetName();
        if (CameraName.Equals(TEXT("Camera"), ESearchCase::IgnoreCase) ||
            CameraName.Contains(TEXT("FirstPerson"), ESearchCase::IgnoreCase))
        {
            FirstPersonCamera = Camera;
            break;
        }
    }

    if (!FirstPersonCamera)
    {
        for (UCameraComponent* Camera : Cameras)
        {
            if (Camera &&
                !Camera->GetName().Contains(TEXT("FollowCamera"), ESearchCase::IgnoreCase))
            {
                FirstPersonCamera = Camera;
                break;
            }
        }
    }

    if (!FirstPersonCamera)
    {
        return;
    }

    // The authored camera was previously left on a high character/head
    // attachment. Anchor it to the capsule at a natural eye height so the
    // view is through the player rather than looking down on the player.
    if (USceneComponent* CharacterRoot = OwnerActor->GetRootComponent())
    {
        if (FirstPersonCamera->GetAttachParent() != CharacterRoot)
        {
            FirstPersonCamera->AttachToComponent(
                CharacterRoot,
                FAttachmentTransformRules::KeepRelativeTransform);
        }

        FirstPersonCamera->SetRelativeLocation(FVector(10.0f, 0.0f, 64.0f));
        FirstPersonCamera->SetRelativeRotation(FRotator::ZeroRotator);
        FirstPersonCamera->bUsePawnControlRotation = true;
    }

    for (UCameraComponent* Camera : Cameras)
    {
        if (Camera)
        {
            Camera->SetActive(Camera == FirstPersonCamera);
        }
    }
}

void UPlayerInventoryComponent::AddGroundFloorBlueprint()
{
    if (bHasGroundFloorBlueprint)
    {
        return;
    }

    bHasGroundFloorBlueprint = true;

    if (AActor* OwnerActor = GetOwner())
    {
        GroundFloorTrackingOrigin = OwnerActor->GetActorLocation();
        bHasGroundFloorTrackingOrigin = true;
    }

    // Give the ground-floor evidence a valid location immediately. Explicit
    // room volumes can still replace this with their authored positions.
    SetBlueprintLocation(
        TEXT("GroundFloor"),
        FText::FromString(TEXT("GROUND FLOOR")),
        FText::FromString(TEXT("CURRENT POSITION")),
        FVector2D(0.5f, 0.5f));

    UpdateHUDBlueprintCount();
}

bool UPlayerInventoryComponent::HasGroundFloorBlueprint() const
{
    return bHasGroundFloorBlueprint;
}

void UPlayerInventoryComponent::AddFirstFloorBlueprint()
{
    if (bHasFirstFloorBlueprint)
    {
        return;
    }

    bHasFirstFloorBlueprint = true;
    UpdateHUDBlueprintCount();

    if (!bHasPhaseRegulatorBlueprint)
    {
        TArray<UUserWidget*> FoundWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
            this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

        for (UUserWidget* FoundWidget : FoundWidgets)
        {
            if (UGameHUDWidget* GameHUD = Cast<UGameHUDWidget>(FoundWidget))
            {
                GameHUD->UpdateObjective(
                    FText::FromString(TEXT("Search the first-floor office")),
                    FText::FromString(
                        TEXT("Use the first-floor blueprint to find the office")));
            }
        }
    }
}

bool UPlayerInventoryComponent::HasFirstFloorBlueprint() const
{
    return bHasFirstFloorBlueprint;
}

void UPlayerInventoryComponent::AddPhaseRegulatorBlueprint()
{
    if (bHasPhaseRegulatorBlueprint)
    {
        return;
    }

    bHasPhaseRegulatorBlueprint = true;
    // The pickup opens this schematic immediately, so the next Tab cycle
    // should return to the first collected floor plan.
    NextEvidenceIndex = 0;
    UpdateHUDBlueprintCount();
    RefreshPhaseRegulatorObjective();
}

bool UPlayerInventoryComponent::HasPhaseRegulatorBlueprint() const
{
    return bHasPhaseRegulatorBlueprint;
}

void UPlayerInventoryComponent::AddMaintenanceTool()
{
    if (bHasMaintenanceTool)
    {
        return;
    }

    bHasMaintenanceTool = true;
    UpdateHUDComponentCount();
}

bool UPlayerInventoryComponent::HasMaintenanceTool() const
{
    return bHasMaintenanceTool;
}

void UPlayerInventoryComponent::AddCalibrationReader()
{
    if (bHasCalibrationReader)
    {
        return;
    }

    bHasCalibrationReader = true;
    UpdateHUDRepairItemIcons();
    RefreshPhaseRegulatorObjective();
}

bool UPlayerInventoryComponent::HasCalibrationReader() const
{
    return bHasCalibrationReader;
}

bool UPlayerInventoryComponent::RemoveCalibrationReader()
{
    if (!bHasCalibrationReader)
    {
        return false;
    }

    bHasCalibrationReader = false;
    UpdateHUDRepairItemIcons();
    return true;
}

void UPlayerInventoryComponent::AddPressureCanister()
{
    if (bHasPressureCanister)
    {
        return;
    }

    bHasPressureCanister = true;
    UpdateHUDRepairItemIcons();
    RefreshPhaseRegulatorObjective();
}

bool UPlayerInventoryComponent::HasPressureCanister() const
{
    return bHasPressureCanister;
}

bool UPlayerInventoryComponent::RemovePressureCanister()
{
    if (!bHasPressureCanister)
    {
        return false;
    }

    bHasPressureCanister = false;
    UpdateHUDRepairItemIcons();
    return true;
}

void UPlayerInventoryComponent::AddResonanceConduit()
{
    if (bHasResonanceConduit)
    {
        return;
    }

    bHasResonanceConduit = true;
    UpdateHUDRepairItemIcons();
    RefreshPhaseRegulatorObjective();
}

bool UPlayerInventoryComponent::HasResonanceConduit() const
{
    return bHasResonanceConduit;
}

bool UPlayerInventoryComponent::RemoveResonanceConduit()
{
    if (!bHasResonanceConduit)
    {
        return false;
    }

    bHasResonanceConduit = false;
    UpdateHUDRepairItemIcons();
    return true;
}

int32 UPlayerInventoryComponent::GetHeldRepairItemCount() const
{
    return
        (bHasCalibrationReader ? 1 : 0) +
        (bHasPressureCanister ? 1 : 0) +
        (bHasResonanceConduit ? 1 : 0);
}

void UPlayerInventoryComponent::RefreshPhaseRegulatorObjective()
{
    if (!bHasPhaseRegulatorBlueprint || !GetWorld())
    {
        return;
    }

    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    if (FoundWidgets.Num() == 0)
    {
        return;
    }

    AGenerator* Generator = Cast<AGenerator>(
        UGameplayStatics::GetActorOfClass(this, AGenerator::StaticClass()));
    APhaseRegulatorMachine* Machine = Cast<APhaseRegulatorMachine>(
        UGameplayStatics::GetActorOfClass(
            this, APhaseRegulatorMachine::StaticClass()));

    const bool bPowerIsOn = Generator && Generator->IsRunning();
    const int32 InstalledParts = Machine ? Machine->GetInstalledPartCount() : 0;
    const int32 FoundParts = FMath::Clamp(
        InstalledParts + GetHeldRepairItemCount(), 0, 3);

    FText Objective;
    FText Hint;

    if (!bPowerIsOn)
    {
        Objective = FText::FromString(TEXT("Restore power to the manor"));
        Hint = FText::FromString(
            TEXT("The Phase Regulator cannot start without power"));
    }
    else if (Machine && Machine->IsMachineOnline())
    {
        Objective = FText::FromString(
            TEXT("Phase Regulator online"));
        Hint = FText::FromString(TEXT("Find the tower blueprint"));
    }
    else if (Machine && Machine->IsMachineStarting())
    {
        Objective = FText::FromString(
            TEXT("Phase Regulator starting..."));
        Hint = FText::FromString(
            TEXT("Wait for the startup sequence to complete"));
    }
    else if (InstalledParts >= 3)
    {
        Objective = FText::FromString(
            TEXT("Activate the repaired Phase Regulator"));
        Hint = FText::FromString(
            TEXT("Return to the machine and press E"));
    }
    else if (FoundParts >= 3)
    {
        Objective = FText::FromString(
            TEXT("Return to the Phase Regulator machine"));
        Hint = FText::Format(
            FText::FromString(TEXT("Install the collected components - {0}/3")),
            FText::AsNumber(InstalledParts));
    }
    else
    {
        Objective = FText::FromString(TEXT("Repair the Phase Regulator"));
        Hint = FText::Format(
            FText::FromString(TEXT("Find the three missing components - {0}/3")),
            FText::AsNumber(FoundParts));
    }

    for (UUserWidget* FoundWidget : FoundWidgets)
    {
        if (UGameHUDWidget* GameHUD = Cast<UGameHUDWidget>(FoundWidget))
        {
            GameHUD->UpdateObjective(Objective, Hint);
        }
    }
}

void UPlayerInventoryComponent::SetBlueprintLocation(
    FName FloorID,
    const FText& FloorName,
    const FText& RoomName,
    FVector2D NormalizedMapPosition)
{
    bHasCurrentBlueprintLocation = !FloorID.IsNone();
    CurrentBlueprintFloorID = FloorID;
    CurrentBlueprintFloorName = FloorName;
    CurrentBlueprintRoomName = RoomName;
    CurrentBlueprintMapPosition = FVector2D(
        FMath::Clamp(NormalizedMapPosition.X, 0.0f, 1.0f),
        FMath::Clamp(NormalizedMapPosition.Y, 0.0f, 1.0f));
}

bool UPlayerInventoryComponent::HasBlueprintLocation() const
{
    return bHasCurrentBlueprintLocation;
}

FName UPlayerInventoryComponent::GetCurrentBlueprintFloorID() const
{
    return CurrentBlueprintFloorID;
}

FText UPlayerInventoryComponent::GetCurrentBlueprintFloorName() const
{
    return CurrentBlueprintFloorName;
}

FText UPlayerInventoryComponent::GetCurrentBlueprintRoomName() const
{
    return CurrentBlueprintRoomName;
}

FVector2D UPlayerInventoryComponent::GetCurrentBlueprintMapPosition() const
{
    if (CurrentBlueprintFloorID == TEXT("GroundFloor") &&
        bHasGroundFloorTrackingOrigin)
    {
        return GetGroundFloorTrackedMapPosition();
    }

    return CurrentBlueprintMapPosition;
}

FVector2D UPlayerInventoryComponent::GetGroundFloorTrackedMapPosition() const
{
    if (!GetOwner())
    {
        return GroundFloorTrackingMapOrigin;
    }

    struct FGroundMapAnchor
    {
        FVector2D World;
        FVector2D Map;
    };

    // Captured in-game at the centre of each ground-floor room/landmark.
    static const FGroundMapAnchor Anchors[] =
    {
        {{-984.98f, -1286.78f}, {0.490f, 0.310f}}, // Lab
        {{-159.93f, -1307.85f}, {0.490f, 0.480f}}, // Living room
        {{283.39f, 24.13f}, {0.340f, 0.580f}},     // Hall entrance
        {{1243.33f, -291.91f}, {0.360f, 0.790f}},  // Dining
        {{1165.53f, -1232.59f}, {0.490f, 0.760f}}, // Kitchen
        {{1049.49f, 388.47f}, {0.280f, 0.740f}},   // South bathroom
        {{163.82f, 1266.44f}, {0.165f, 0.660f}},   // Front entrance
        {{-648.63f, 1338.83f}, {0.165f, 0.550f}},  // Front terrace
        {{-484.61f, 815.37f}, {0.220f, 0.420f}},   // Stairs
        {{-525.45f, 7.51f}, {0.340f, 0.420f}},     // Camera room
        {{-1042.55f, -138.22f}, {0.340f, 0.320f}}, // Computer room
        {{-1430.58f, 1485.80f}, {0.150f, 0.250f}}, // Garage
        {{-846.34f, -791.27f}, {0.420f, 0.360f}},  // Central hall
        {{-1250.38f, -811.06f}, {0.430f, 0.280f}}, // North bathroom
        {{456.53f, -1901.09f}, {0.620f, 0.580f}},  // East terrace
        {{-889.16f, -4426.04f}, {0.880f, 0.350f}}, // Pool house
        {{357.63f, -4485.85f}, {0.880f, 0.630f}},  // Pool
        {{1816.11f, -4329.39f}, {0.880f, 0.860f}}, // Generator house
        {{-1894.77f, -1554.91f}, {0.450f, 0.090f}} // Energy tower
    };

    const FVector OwnerLocation = GetOwner()->GetActorLocation();
    const FVector2D WorldPosition(OwnerLocation.X, OwnerLocation.Y);

    // Blend the four closest captured anchors. This stays exact at room
    // centres while remaining smooth through doors, halls, and terraces.
    float NearestDistanceSq[4] = {MAX_flt, MAX_flt, MAX_flt, MAX_flt};
    int32 NearestIndex[4] = {INDEX_NONE, INDEX_NONE, INDEX_NONE, INDEX_NONE};

    for (int32 AnchorIndex = 0;
         AnchorIndex < UE_ARRAY_COUNT(Anchors);
         ++AnchorIndex)
    {
        const float DistanceSq = FVector2D::DistSquared(
            WorldPosition, Anchors[AnchorIndex].World);
        if (DistanceSq < 1.0f)
        {
            return Anchors[AnchorIndex].Map;
        }

        for (int32 Slot = 0; Slot < 4; ++Slot)
        {
            if (DistanceSq < NearestDistanceSq[Slot])
            {
                for (int32 Shift = 3; Shift > Slot; --Shift)
                {
                    NearestDistanceSq[Shift] = NearestDistanceSq[Shift - 1];
                    NearestIndex[Shift] = NearestIndex[Shift - 1];
                }
                NearestDistanceSq[Slot] = DistanceSq;
                NearestIndex[Slot] = AnchorIndex;
                break;
            }
        }
    }

    FVector2D WeightedMap = FVector2D::ZeroVector;
    float TotalWeight = 0.0f;
    for (int32 Slot = 0; Slot < 4; ++Slot)
    {
        if (NearestIndex[Slot] == INDEX_NONE)
        {
            continue;
        }
        const float Weight = 1.0f / FMath::Max(NearestDistanceSq[Slot], 1.0f);
        WeightedMap += Anchors[NearestIndex[Slot]].Map * Weight;
        TotalWeight += Weight;
    }

    return TotalWeight > 0.0f
        ? WeightedMap / TotalWeight
        : GroundFloorTrackingMapOrigin;
}

bool UPlayerInventoryComponent::IsPlayerOnGroundFloor() const
{
    return GetOwner() &&
        GetOwner()->GetActorLocation().Z < FirstFloorMinimumPlayerZ;
}

namespace
{
    struct FFirstFloorMapAnchor
    {
        FVector2D World;
        FVector2D Map;
        const TCHAR* RoomName;
    };

    static const FFirstFloorMapAnchor FirstFloorAnchors[] =
    {
        {{-656.20f, 506.07f}, {0.195f, 0.269f}, TEXT("STAIRS")},
        {{-951.57f, -201.49f}, {0.319f, 0.125f}, TEXT("BEDROOM 1")},
        {{-1030.97f, -1273.40f}, {0.523f, 0.125f}, TEXT("BEDROOM 2")},
        {{-205.89f, -1341.84f}, {0.519f, 0.344f}, TEXT("BEDROOM 3")},
        {{224.11f, -113.86f}, {0.340f, 0.465f}, TEXT("MAIN HALL")},
        {{1304.22f, -259.85f}, {0.361f, 0.751f}, TEXT("OFFICE")},
        {{1224.19f, -1228.07f}, {0.518f, 0.701f}, TEXT("LIBRARY")},
        {{710.53f, 699.37f}, {0.219f, 0.609f}, TEXT("NORTH STUDY")},
        {{1457.27f, 714.82f}, {0.214f, 0.782f}, TEXT("SOUTH STUDY")},
        {{633.18f, 1377.99f}, {0.112f, 0.690f}, TEXT("WEST BALCONY")},
        {{475.41f, -1448.02f}, {0.584f, 0.525f}, TEXT("EAST BALCONY")},
        {{1783.46f, -1246.29f}, {0.510f, 0.865f}, TEXT("LIBRARY BALCONY")},
        {{-1212.60f, -770.50f}, {0.423f, 0.075f}, TEXT("BEDROOM 2 BATHROOM")},
        {{-514.83f, -41.83f}, {0.294f, 0.269f}, TEXT("BEDROOM 1 BATHROOM")},
        {{1045.63f, 399.17f}, {0.258f, 0.687f}, TEXT("STUDY BATHROOM")},
        {{289.70f, -79.95f}, {0.375f, 0.465f}, TEXT("MAIN HALL")}
    };

    void FindNearestFirstFloorAnchors(
        const FVector2D& WorldPosition,
        float (&OutDistanceSq)[4],
        int32 (&OutIndex)[4])
    {
        for (int32 Slot = 0; Slot < 4; ++Slot)
        {
            OutDistanceSq[Slot] = MAX_flt;
            OutIndex[Slot] = INDEX_NONE;
        }

        for (int32 AnchorIndex = 0;
             AnchorIndex < UE_ARRAY_COUNT(FirstFloorAnchors);
             ++AnchorIndex)
        {
            const float DistanceSq = FVector2D::DistSquared(
                WorldPosition, FirstFloorAnchors[AnchorIndex].World);
            for (int32 Slot = 0; Slot < 4; ++Slot)
            {
                if (DistanceSq < OutDistanceSq[Slot])
                {
                    for (int32 Shift = 3; Shift > Slot; --Shift)
                    {
                        OutDistanceSq[Shift] = OutDistanceSq[Shift - 1];
                        OutIndex[Shift] = OutIndex[Shift - 1];
                    }
                    OutDistanceSq[Slot] = DistanceSq;
                    OutIndex[Slot] = AnchorIndex;
                    break;
                }
            }
        }
    }
}

FVector2D UPlayerInventoryComponent::GetFirstFloorTrackedMapPosition() const
{
    if (!GetOwner())
    {
        return FVector2D(0.5f, 0.5f);
    }

    const FVector Location = GetOwner()->GetActorLocation();
    const FVector2D WorldPosition(Location.X, Location.Y);
    float DistanceSq[4];
    int32 Index[4];
    FindNearestFirstFloorAnchors(WorldPosition, DistanceSq, Index);

    if (DistanceSq[0] < 1.0f)
    {
        return FirstFloorAnchors[Index[0]].Map;
    }

    FVector2D WeightedMap = FVector2D::ZeroVector;
    float TotalWeight = 0.0f;
    for (int32 Slot = 0; Slot < 4; ++Slot)
    {
        if (Index[Slot] == INDEX_NONE)
        {
            continue;
        }
        const float Weight = 1.0f / FMath::Max(DistanceSq[Slot], 1.0f);
        WeightedMap += FirstFloorAnchors[Index[Slot]].Map * Weight;
        TotalWeight += Weight;
    }

    return TotalWeight > 0.0f
        ? WeightedMap / TotalWeight
        : FVector2D(0.5f, 0.5f);
}

FText UPlayerInventoryComponent::GetFirstFloorTrackedRoomName() const
{
    if (!GetOwner())
    {
        return FText::FromString(TEXT("FIRST FLOOR"));
    }

    const FVector Location = GetOwner()->GetActorLocation();
    float DistanceSq[4];
    int32 Index[4];
    FindNearestFirstFloorAnchors(
        FVector2D(Location.X, Location.Y), DistanceSq, Index);
    return Index[0] != INDEX_NONE
        ? FText::FromString(FirstFloorAnchors[Index[0]].RoomName)
        : FText::FromString(TEXT("FIRST FLOOR"));
}

bool UPlayerInventoryComponent::IsPlayerOnFirstFloor() const
{
    return GetOwner() &&
        GetOwner()->GetActorLocation().Z >= FirstFloorMinimumPlayerZ;
}

void UPlayerInventoryComponent::UpdateHUDRepairItemIcons()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    for (UUserWidget* FoundWidget : FoundWidgets)
    {
        if (UGameHUDWidget* GameHUD = Cast<UGameHUDWidget>(FoundWidget))
        {
            GameHUD->UpdateRepairItemIcons(
                bHasCalibrationReader,
                bHasPressureCanister,
                bHasResonanceConduit);
        }
    }
}

void UPlayerInventoryComponent::AddGeneratorFuse()
{
    if (bHasGeneratorFuse)
    {
        return;
    }

    bHasGeneratorFuse = true;
    UpdateHUDComponentCount();
}

bool UPlayerInventoryComponent::RemoveGeneratorFuse()
{
    if (!bHasGeneratorFuse)
    {
        return false;
    }

    bHasGeneratorFuse = false;
    UpdateHUDComponentCount();

    return true;
}

bool UPlayerInventoryComponent::HasGeneratorFuse() const
{
    return bHasGeneratorFuse;
}

int32 UPlayerInventoryComponent::GetComponentCount() const
{
    int32 Count = 0;

    if (bHasMaintenanceTool)
    {
        ++Count;
    }

    if (bHasGeneratorFuse)
    {
        ++Count;
    }

    return Count;
}

void UPlayerInventoryComponent::UpdateHUDComponentCount()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    if (FoundWidgets.Num() == 0)
    {
        return;
    }

    UGameHUDWidget* GameHUD =
        Cast<UGameHUDWidget>(FoundWidgets[0]);

    if (GameHUD)
    {
        GameHUD->UpdateComponentCount(GetComponentCount(), 4);
    }
}

void UPlayerInventoryComponent::UpdateHUDBlueprintCount()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    int32 BlueprintCount = 0;

    if (bHasGroundFloorBlueprint)
    {
        ++BlueprintCount;
    }

    if (bHasPhaseRegulatorBlueprint)
    {
        ++BlueprintCount;
    }

    if (bHasFirstFloorBlueprint)
    {
        ++BlueprintCount;
    }

    // More than one HUD can briefly exist during level transitions or PIE.
    // Updating only index zero can leave the visible HUD with stale values.
    for (UUserWidget* FoundWidget : FoundWidgets)
    {
        if (UGameHUDWidget* GameHUD = Cast<UGameHUDWidget>(FoundWidget))
        {
            GameHUD->UpdateBlueprintCount(BlueprintCount, 3);
        }
    }
}

void UPlayerInventoryComponent::OpenEvidence()
{
    if (!bHasGroundFloorBlueprint &&
        !bHasFirstFloorBlueprint &&
        !bHasPhaseRegulatorBlueprint)
    {
        return;
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    if (CurrentEvidenceWidget &&
        CurrentEvidenceWidget->IsInViewport())
    {
        return;
    }

    // Stable evidence order: ground floor -> first floor -> Phase Regulator.
    // Missing or unconfigured documents are skipped safely.
    for (int32 Attempt = 0; Attempt < 3; ++Attempt)
    {
        const int32 EvidenceIndex = (NextEvidenceIndex + Attempt) % 3;
        if (IsEvidenceIndexCollected(EvidenceIndex) &&
            TryOpenEvidenceIndex(EvidenceIndex))
        {
            NextEvidenceIndex = (EvidenceIndex + 1) % 3;
            return;
        }
    }
}

void UPlayerInventoryComponent::CloseEvidence()
{
    if (!CurrentEvidenceWidget || !CurrentEvidenceWidget->IsInViewport())
    {
        return;
    }

    CurrentEvidenceWidget->RemoveFromParent();
    CurrentEvidenceWidget = nullptr;

    APlayerController* PlayerController = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;
    if (PlayerController)
    {
        PlayerController->bShowMouseCursor = false;
        PlayerController->SetInputMode(FInputModeGameOnly());
    }

    ForceFirstPersonCamera();
}

void UPlayerInventoryComponent::OpenFirstFloorEvidence()
{
    if (bHasFirstFloorBlueprint && TryOpenEvidenceIndex(1))
    {
        NextEvidenceIndex = 2;
    }
}

bool UPlayerInventoryComponent::IsEvidenceIndexCollected(int32 EvidenceIndex) const
{
    switch (EvidenceIndex)
    {
        case 0: return bHasGroundFloorBlueprint;
        case 1: return bHasFirstFloorBlueprint;
        case 2: return bHasPhaseRegulatorBlueprint;
        default: return false;
    }
}

bool UPlayerInventoryComponent::TryOpenEvidenceIndex(int32 EvidenceIndex)
{
    APlayerController* PlayerController = GetWorld()
        ? GetWorld()->GetFirstPlayerController()
        : nullptr;
    if (!PlayerController)
    {
        return false;
    }

    TSubclassOf<UUserWidget> WidgetClass;
    if (EvidenceIndex == 0)
    {
        WidgetClass = EvidenceWidgetClass;
    }
    else if (EvidenceIndex == 1)
    {
        WidgetClass = FirstFloorEvidenceWidgetClass
            ? FirstFloorEvidenceWidgetClass
            : EvidenceWidgetClass;
    }
    else if (EvidenceIndex == 2)
    {
        WidgetClass = PhaseRegulatorEvidenceWidgetClass;
    }

    if (!WidgetClass)
    {
        return false;
    }

    if ((EvidenceIndex == 0 || EvidenceIndex == 1) && GetOwner())
    {
        const FVector PlayerLocation = GetOwner()->GetActorLocation();
        const FVector2D PredictedMapPosition = EvidenceIndex == 0
            ? GetGroundFloorTrackedMapPosition()
            : GetCurrentBlueprintMapPosition();
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("%s_MAP_CALIBRATION World=(%.2f, %.2f, %.2f) Map=(%.4f, %.4f) Room=\"%s\""),
            EvidenceIndex == 0 ? TEXT("GROUND") : TEXT("FIRST_FLOOR"),
            PlayerLocation.X,
            PlayerLocation.Y,
            PlayerLocation.Z,
            PredictedMapPosition.X,
            PredictedMapPosition.Y,
            *GetCurrentBlueprintRoomName().ToString());
    }

    CurrentEvidenceWidget = CreateWidget<UUserWidget>(
        PlayerController,
        WidgetClass);
    if (!CurrentEvidenceWidget)
    {
        return false;
    }

    if (UEvidenceWidget* EvidenceWidget =
            Cast<UEvidenceWidget>(CurrentEvidenceWidget))
    {
        if (EvidenceIndex == 0)
        {
            EvidenceWidget->ConfigureFloorPlan(
                TEXT("GroundFloor"), nullptr, true);
        }
        else if (EvidenceIndex == 1)
        {
            EvidenceWidget->ConfigureFloorPlan(
                TEXT("FirstFloor"),
                FirstFloorBlueprintTexture.LoadSynchronous(),
                true);
        }
    }

    CurrentEvidenceWidget->AddToViewport();
    PlayerController->bShowMouseCursor = true;

    FInputModeGameAndUI InputMode;
    InputMode.SetWidgetToFocus(CurrentEvidenceWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(
        EMouseLockMode::DoNotLock);
    PlayerController->SetInputMode(InputMode);

    return true;
}

void UPlayerInventoryComponent::AddFlashlight()
{
    if (bHasFlashlight)
    {
        return;
    }

    bHasFlashlight = true;
}

bool UPlayerInventoryComponent::HasFlashlight() const
{
    return bHasFlashlight;
}

bool UPlayerInventoryComponent::AddKey(FName KeyID)
{
    if (KeyID.IsNone() ||
        CollectedKeyIDs.Contains(KeyID))
    {
        return false;
    }

    CollectedKeyIDs.Add(KeyID);
    UpdateHUDKeyCount();

    return true;
}

bool UPlayerInventoryComponent::HasKey(FName KeyID) const
{
    if (KeyID.IsNone())
    {
        return false;
    }

    return CollectedKeyIDs.Contains(KeyID);
}

int32 UPlayerInventoryComponent::GetKeyCount() const
{
    return CollectedKeyIDs.Num();
}

void UPlayerInventoryComponent::UpdateHUDKeyCount()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this, FoundWidgets, UGameHUDWidget::StaticClass(), false);

    if (FoundWidgets.Num() == 0)
    {
        return;
    }

    UGameHUDWidget* GameHUD =
        Cast<UGameHUDWidget>(FoundWidgets[0]);

    if (GameHUD)
    {
        GameHUD->UpdateKeyCount(GetKeyCount());
    }
}
