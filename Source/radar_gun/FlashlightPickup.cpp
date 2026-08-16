#include "FlashlightPickup.h"

#include "GameHUDWidget.h"
#include "PlayerInventoryComponent.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

AFlashlightPickup::AFlashlightPickup()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    FlashlightMesh =
        CreateDefaultSubobject<UStaticMeshComponent>(
            TEXT("FlashlightMesh"));

    RootComponent = FlashlightMesh;

    FlashlightMesh->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    FlashlightLight =
        CreateDefaultSubobject<USpotLightComponent>(
            TEXT("FlashlightLight"));

    FlashlightLight->SetupAttachment(FlashlightMesh);

    FlashlightLight->SetVisibility(true);
    FlashlightLight->SetIntensity(8000.0f);
    FlashlightLight->SetAttenuationRadius(2500.0f);
    FlashlightLight->SetInnerConeAngle(15.0f);
    FlashlightLight->SetOuterConeAngle(30.0f);

    InteractionTrigger =
        CreateDefaultSubobject<UBoxComponent>(
            TEXT("InteractionTrigger"));

    InteractionTrigger->SetupAttachment(RootComponent);
    InteractionTrigger->SetBoxExtent(
        FVector(100.0f, 100.0f, 100.0f));

    InteractionTrigger->SetCollisionProfileName(
        TEXT("Trigger"));
}

void AFlashlightPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bCollected || !IsValid(OwningCharacter))
    {
        return;
    }

    APlayerController* PlayerController =
        Cast<APlayerController>(OwningCharacter->GetController());

    if (!PlayerController || !PlayerController->PlayerCameraManager)
    {
        return;
    }

    // Keep the flashlight actor and mesh driven entirely by the hand socket.
    // Only the spotlight beam follows the camera/mouse aim.
    const FRotator TargetRotation =
        (PlayerController->PlayerCameraManager->GetCameraRotation().Quaternion()
            * CameraAimOffset.Quaternion()).Rotator();

    const FRotator NewRotation = FMath::RInterpTo(
        FlashlightLight->GetComponentRotation(),
        TargetRotation,
        DeltaTime,
        AimRotationSpeed);

    FlashlightLight->SetWorldRotation(
        NewRotation,
        false,
        nullptr,
        ETeleportType::TeleportPhysics);
}

void AFlashlightPickup::BeginPlay()
{
    Super::BeginPlay();

    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(
        this,
        &AFlashlightPickup::OnPlayerEnterRange);

    InteractionTrigger->OnComponentEndOverlap.AddDynamic(
        this,
        &AFlashlightPickup::OnPlayerLeaveRange);

    FindGameHUD();
}

void AFlashlightPickup::FindGameHUD()
{
    TArray<UUserWidget*> FoundWidgets;

    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(
        this,
        FoundWidgets,
        UGameHUDWidget::StaticClass(),
        false);

    if (FoundWidgets.Num() > 0)
    {
        GameHUD = Cast<UGameHUDWidget>(FoundWidgets[0]);
    }
}

void AFlashlightPickup::OnPlayerEnterRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!bCollected &&
        PlayerController &&
        OtherActor == PlayerController->GetPawn())
    {
        bPlayerIsNearby = true;

        if (!GameHUD)
        {
            FindGameHUD();
        }

        if (GameHUD)
        {
            GameHUD->ShowInteractionPrompt();
        }

        EnableInput(PlayerController);

        if (InputComponent)
        {
            InputComponent->BindKey(
                EKeys::E,
                IE_Pressed,
                this,
                &AFlashlightPickup::CollectFlashlight);

            InputComponent->BindKey(
                EKeys::Gamepad_FaceButton_Left,
                IE_Pressed,
                this,
                &AFlashlightPickup::CollectFlashlight);
        }
    }
}

void AFlashlightPickup::OnPlayerLeaveRange(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex)
{
    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!bCollected &&
        PlayerController &&
        OtherActor == PlayerController->GetPawn())
    {
        bPlayerIsNearby = false;

        if (GameHUD)
        {
            GameHUD->HideInteractionPrompt();
        }

        DisableInput(PlayerController);
    }
}

void AFlashlightPickup::CollectFlashlight()
{
    if (!bPlayerIsNearby || bCollected)
    {
        return;
    }

    APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();

    if (!PlayerController)
    {
        return;
    }

    ACharacter* PlayerCharacter =
        Cast<ACharacter>(PlayerController->GetPawn());

    if (!PlayerCharacter || !PlayerCharacter->GetMesh())
    {
        return;
    }

    UPlayerInventoryComponent* Inventory =
        PlayerCharacter->FindComponentByClass<
        UPlayerInventoryComponent>();

    if (!Inventory)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("PlayerInventoryComponent not found."));

        return;
    }

    Inventory->AddFlashlight();

    bCollected = true;
    bFlashlightOn = true;

    if (GameHUD)
    {
        GameHUD->HideInteractionPrompt();
        GameHUD->ShowFlashlightHint();
    }

    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            PickupSound,
            GetActorLocation());
    }

    InteractionTrigger->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    AttachToComponent(
        PlayerCharacter->GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("FlashlightSocket"));

    OwningCharacter = PlayerCharacter;
    SetActorTickEnabled(true);

    FlashlightMesh->SetVisibility(true);
    FlashlightLight->SetVisibility(true);

    /*
     * Input stays enabled after collection so F can toggle
     * the flashlight.
     */
    if (InputComponent)
    {
        InputComponent->BindKey(
            EKeys::F,
            IE_Pressed,
            this,
            &AFlashlightPickup::ToggleFlashlight);
    }

    UE_LOG(LogTemp, Warning, TEXT("Flashlight collected and switched ON."));
}

void AFlashlightPickup::ToggleFlashlight()
{
    if (!bCollected)
    {
        return;
    }

    bFlashlightOn = !bFlashlightOn;

    FlashlightLight->SetVisibility(bFlashlightOn);

    if (ToggleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            ToggleSound,
            GetActorLocation());
    }

    UE_LOG(
        LogTemp,
        Warning,
        TEXT("Flashlight %s"),
        bFlashlightOn ? TEXT("ON") : TEXT("OFF"));
}
