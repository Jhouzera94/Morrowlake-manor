#include "MansionStartupSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "GameHUDWidget.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

bool UMansionStartupSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    return World &&
        (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UMansionStartupSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    if (!InWorld.GetMapName().Contains(TEXT("Abandoned_Story")))
    {
        return;
    }

    RemainingAttempts = 30;
    UE_LOG(LogTemp, Warning, TEXT("MansionStartup: repair system active"));
    InWorld.GetTimerManager().SetTimer(
        RepairTimer,
        this,
        &UMansionStartupSubsystem::RepairStartup,
        0.1f,
        true,
        0.0f);
}

void UMansionStartupSubsystem::RepairStartup()
{
    UWorld* World = GetWorld();
    APlayerController* PC = World ? UGameplayStatics::GetPlayerController(World, 0) : nullptr;
    APawn* Pawn = PC ? PC->GetPawn() : nullptr;

    if (!World || !PC)
    {
        if (--RemainingAttempts <= 0 && World)
        {
            World->GetTimerManager().ClearTimer(RepairTimer);
        }
        return;
    }

    if (!Pawn)
    {
        static TSubclassOf<APawn> PlayerClass = LoadClass<APawn>(
            nullptr,
            TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter.BP_ThirdPersonCharacter_C"));
        if (PlayerClass)
        {
            FActorSpawnParameters Params;
            Params.SpawnCollisionHandlingOverride =
                ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            Pawn = World->SpawnActor<APawn>(
                PlayerClass,
                FVector(-1317.85, -1092.33, 291.97),
                FRotator(0.0, 125.35, 0.0),
                Params);
            if (Pawn)
            {
                PC->Possess(Pawn);
                UE_LOG(LogTemp, Warning, TEXT("MansionStartup: spawned and possessed player"));
            }
        }
    }

    if (Pawn)
    {
        const FVector SafeLocation(-1317.85, -1092.33, 291.97);
        Pawn->SetActorLocationAndRotation(
            SafeLocation,
            FRotator(0.0, 125.35, 0.0),
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        UE_LOG(LogTemp, Warning, TEXT("MansionStartup: player placed at mansion entrance"));

        if (ACharacter* Character = Cast<ACharacter>(Pawn))
        {
            // Mouse/controller yaw turns the whole body. The camera and
            // flashlight can still use pitch independently for looking up/down.
            Character->bUseControllerRotationYaw = true;

            if (UCharacterMovementComponent* Movement =
                    Character->GetCharacterMovement())
            {
                Movement->bOrientRotationToMovement = false;
                Movement->bUseControllerDesiredRotation = false;
            }
        }
    }

    if (PC->IsLocalController())
    {
        TArray<UUserWidget*> ExistingHUDs;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
            World, ExistingHUDs, UGameHUDWidget::StaticClass(), false);

        if (ExistingHUDs.IsEmpty())
        {
            static TSubclassOf<UUserWidget> HUDClass = LoadClass<UUserWidget>(
                nullptr,
                TEXT("/Game/Abandoned_MAnsion/UI/WBP_HUD.WBP_HUD_C"));
            if (HUDClass)
            {
                if (UUserWidget* HUD = CreateWidget<UUserWidget>(PC, HUDClass))
                {
                    HUD->AddToViewport(10);
                    UE_LOG(LogTemp, Warning, TEXT("MansionStartup: HUD created"));
                }
            }
        }
    }

    if (Pawn)
    {
        if (!World->GetTimerManager().IsTimerActive(ControllerLookTimer))
        {
            World->GetTimerManager().SetTimer(
                ControllerLookTimer,
                this,
                &UMansionStartupSubsystem::UpdateControllerLook,
                0.01f,
                true);
        }

        World->GetTimerManager().ClearTimer(RepairTimer);
    }
}

void UMansionStartupSubsystem::UpdateControllerLook()
{
    UWorld* World = GetWorld();
    APlayerController* PC = World
        ? UGameplayStatics::GetPlayerController(World, 0)
        : nullptr;

    if (!PC || !PC->IsLocalController() || PC->IsLookInputIgnored())
    {
        return;
    }

    FVector2D Stick(
        PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX),
        PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY));

    const float Magnitude = Stick.Size();
    if (Magnitude <= ControllerLookDeadZone)
    {
        return;
    }

    // Rescale the usable stick range after removing the radial dead zone.
    Stick = Stick.GetSafeNormal()
        * FMath::Clamp(
            (Magnitude - ControllerLookDeadZone)
                / (1.0f - ControllerLookDeadZone),
            0.0f,
            1.0f);

    FRotator Rotation = PC->GetControlRotation();
    Rotation.Yaw += Stick.X * ControllerYawSpeed * 0.01f;
    Rotation.Pitch = FMath::Clamp(
        FRotator::NormalizeAxis(Rotation.Pitch)
            - Stick.Y * ControllerPitchSpeed * 0.01f,
        -80.0f,
        80.0f);
    Rotation.Roll = 0.0f;
    PC->SetControlRotation(Rotation);
}
