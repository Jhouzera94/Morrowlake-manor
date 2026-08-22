#include "InteractivePC.h"

#include "Generator.h"
#include "GameHUDWidget.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

AInteractivePC::AInteractivePC()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
    InteractionTrigger->SetupAttachment(SceneRoot);
    InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));
}

void AInteractivePC::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    InteractionTrigger->SetBoxExtent(FVector(InteractionDistance));
}

void AInteractivePC::BeginPlay()
{
    Super::BeginPlay();
    BootState = EPCBootState::Off;
    InteractionTrigger->SetBoxExtent(FVector(InteractionDistance));
    InteractionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AInteractivePC::OnPlayerEnter);
    InteractionTrigger->OnComponentEndOverlap.AddDynamic(this, &AInteractivePC::OnPlayerLeave);

    if (!PowerGenerator)
    {
        PowerGenerator = Cast<AGenerator>(UGameplayStatics::GetActorOfClass(this, AGenerator::StaticClass()));
    }

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        EnableInput(PC);
        if (InputComponent)
        {
            InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AInteractivePC::Interact);
            InputComponent->BindKey(EKeys::Gamepad_FaceButton_Left, IE_Pressed, this, &AInteractivePC::Interact);
        }
        DisableInput(PC);
    }

    if (bAutoDiscoverNearbyMonitors)
    {
        AutoDiscoverMonitorActors();
    }
    CreateMonitorMaterials();
    SetAllMonitors(PoweredOffTexture, 0.0f, 0.0f);
}

void AInteractivePC::OnPlayerEnter(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && OtherActor == PC->GetPawn())
    {
        bPlayerNearby = true;
        EnableInput(PC);
        RefreshPrompt();
    }
}

void AInteractivePC::OnPlayerLeave(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && OtherActor == PC->GetPawn())
    {
        bPlayerNearby = false;
        DisableInput(PC);
        HidePrompt();
    }
}

bool AInteractivePC::HasMansionPower() const
{
    return PowerGenerator && PowerGenerator->IsRunning();
}

void AInteractivePC::Interact()
{
    if (!bPlayerNearby || BootState != EPCBootState::Off)
    {
        return;
    }
    if (!HasMansionPower())
    {
        SetAllMonitors(PoweredOffTexture, 0.0f, 0.0f);
        RefreshPrompt();
        return;
    }

    BootState = EPCBootState::Booting;
    if (PCStartupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PCStartupSound, GetActorLocation());
    }
    if (MonitorStartupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, MonitorStartupSound, GetActorLocation());
    }
    SetAllMonitors(BootTexture, PoweredOnEmissiveStrength, 1.0f);
    RefreshPrompt();
    GetWorldTimerManager().SetTimer(BootTimer, this, &AInteractivePC::FinishBoot, BootDuration, false);
}

void AInteractivePC::FinishBoot()
{
    if (!HasMansionPower())
    {
        BootState = EPCBootState::Off;
        SetAllMonitors(PoweredOffTexture, 0.0f, 0.0f);
    }
    else
    {
        BootState = EPCBootState::On;
        SetFinalMonitorTextures();
    }
    RefreshPrompt();
}

void AInteractivePC::CreateMonitorMaterials()
{
    MonitorMaterials.SetNumZeroed(Monitors.Num());
    for (int32 Index = 0; Index < Monitors.Num(); ++Index)
    {
        const FPCMonitorTarget& Target = Monitors[Index];
        if (UStaticMeshComponent* Mesh = ResolveMonitorMesh(Target))
        {
            int32 ResolvedMaterialIndex = Target.MaterialIndex;
            if (ResolvedMaterialIndex < 0 || ResolvedMaterialIndex >= Mesh->GetNumMaterials())
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("InteractivePC '%s': monitor %d requested material slot %d, but %s has %d slot(s). Using slot 0."),
                    *GetName(), Index, ResolvedMaterialIndex, *Mesh->GetPathName(), Mesh->GetNumMaterials());
                ResolvedMaterialIndex = 0;
            }

            MonitorMaterials[Index] = Mesh->CreateAndSetMaterialInstanceDynamic(ResolvedMaterialIndex);
            if (!MonitorMaterials[Index])
            {
                UE_LOG(LogTemp, Error,
                    TEXT("InteractivePC '%s': failed to create a dynamic material for monitor %d (%s), slot %d."),
                    *GetName(), Index, *Mesh->GetPathName(), ResolvedMaterialIndex);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error,
                TEXT("InteractivePC '%s': monitor %d could not resolve actor/component '%s'."),
                *GetName(), Index, *Target.ComponentName.ToString());
        }
    }
}

void AInteractivePC::AutoDiscoverMonitorActors()
{
    struct FScreenCandidate
    {
        AActor* Actor = nullptr;
        UStaticMeshComponent* Mesh = nullptr;
        float DistanceSquared = 0.0f;
        bool bSameFolder = false;
        FString SortName;
    };

    TArray<FScreenCandidate> Candidates;
    const float RadiusSquared = FMath::Square(MonitorDiscoveryRadius);

#if WITH_EDITOR
    const FName ControllerFolder = GetFolderPath();
#endif

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* CandidateActor = *It;
        if (!CandidateActor || CandidateActor == this || CandidateActor->IsA<AInteractivePC>())
        {
            continue;
        }

        FString CandidateName = CandidateActor->GetName();
#if WITH_EDITOR
        CandidateName += TEXT(" ") + CandidateActor->GetActorLabel();
#endif
        if (!CandidateName.Contains(TEXT("SCREEN"), ESearchCase::IgnoreCase)
            && !CandidateName.Contains(TEXT("MONITOR"), ESearchCase::IgnoreCase))
        {
            continue;
        }

        const float DistanceSquared = FVector::DistSquared(
            GetActorLocation(), CandidateActor->GetActorLocation());
        if (DistanceSquared > RadiusSquared)
        {
            continue;
        }

        TInlineComponentArray<UStaticMeshComponent*> Meshes;
        CandidateActor->GetComponents(Meshes);
        if (Meshes.IsEmpty())
        {
            continue;
        }

        FScreenCandidate& Candidate = Candidates.AddDefaulted_GetRef();
        Candidate.Actor = CandidateActor;
        Candidate.Mesh = Meshes[0];
        Candidate.DistanceSquared = DistanceSquared;
        Candidate.SortName = CandidateName;
#if WITH_EDITOR
        Candidate.bSameFolder = !ControllerFolder.IsNone()
            && CandidateActor->GetFolderPath() == ControllerFolder;
#endif
    }

    Candidates.Sort([](const FScreenCandidate& A, const FScreenCandidate& B)
    {
        if (A.bSameFolder != B.bSameFolder)
        {
            return A.bSameFolder;
        }
        if (!FMath::IsNearlyEqual(A.DistanceSquared, B.DistanceSquared))
        {
            return A.DistanceSquared < B.DistanceSquared;
        }
        return A.SortName < B.SortName;
    });

    const int32 DesiredCount = Monitors.Num() > 0 ? Monitors.Num() : 7;
    if (Monitors.Num() < DesiredCount)
    {
        Monitors.SetNum(DesiredCount);
    }

    TSet<AActor*> AlreadyAssignedActors;
    int32 ResolvedCount = 0;
    for (const FPCMonitorTarget& Monitor : Monitors)
    {
        if (ResolveMonitorMesh(Monitor))
        {
            AlreadyAssignedActors.Add(Monitor.Actor);
            ++ResolvedCount;
        }
    }

    int32 CandidateIndex = 0;
    for (int32 Index = 0; Index < DesiredCount; ++Index)
    {
        // A valid manual assignment is authoritative. Discovery only repairs a
        // missing or invalid entry and never replaces the user's chosen actor.
        if (ResolveMonitorMesh(Monitors[Index]))
        {
            continue;
        }

        while (Candidates.IsValidIndex(CandidateIndex)
            && AlreadyAssignedActors.Contains(Candidates[CandidateIndex].Actor))
        {
            ++CandidateIndex;
        }
        if (!Candidates.IsValidIndex(CandidateIndex))
        {
            continue;
        }

        Monitors[Index].Actor = Candidates[CandidateIndex].Actor;
        Monitors[Index].ComponentName = Candidates[CandidateIndex].Mesh->GetFName();
        AlreadyAssignedActors.Add(Candidates[CandidateIndex].Actor);
        ++ResolvedCount;
        UE_LOG(LogTemp, Display,
            TEXT("InteractivePC '%s': monitor %d auto-bound to %s (%s)."),
            *GetName(), Index, *Candidates[CandidateIndex].Actor->GetName(),
            *Candidates[CandidateIndex].Mesh->GetName());
        ++CandidateIndex;
    }

    if (ResolvedCount < DesiredCount)
    {
        UE_LOG(LogTemp, Error,
            TEXT("InteractivePC '%s': found only %d of %d screens within %.0f cm. Check its Outliner folder or increase MonitorDiscoveryRadius."),
            *GetName(), ResolvedCount, DesiredCount, MonitorDiscoveryRadius);
    }
}

UStaticMeshComponent* AInteractivePC::ResolveMonitorMesh(const FPCMonitorTarget& Target) const
{
    AActor* TargetOwner = Target.Actor ? Target.Actor : const_cast<AInteractivePC*>(this);
    TInlineComponentArray<UStaticMeshComponent*> Meshes;
    TargetOwner->GetComponents(Meshes);
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (Target.ComponentName.IsNone() || Mesh->GetFName() == Target.ComponentName)
        {
            return Mesh;
        }
    }
    return nullptr;
}

void AInteractivePC::SetAllMonitors(UTexture* Texture, float EmissiveStrength, float ScreenPower)
{
    for (UMaterialInstanceDynamic* Material : MonitorMaterials)
    {
        if (Material)
        {
            Material->SetTextureParameterValue(TextureParameterName, Texture);
            Material->SetScalarParameterValue(EmissiveParameterName, EmissiveStrength);
            Material->SetScalarParameterValue(ScreenPowerParameterName, ScreenPower);
        }
    }
}

void AInteractivePC::SetFinalMonitorTextures()
{
    for (int32 Index = 0; Index < MonitorMaterials.Num(); ++Index)
    {
        if (UMaterialInstanceDynamic* Material = MonitorMaterials[Index])
        {
            Material->SetTextureParameterValue(TextureParameterName, Monitors[Index].FinalTexture);
            Material->SetScalarParameterValue(EmissiveParameterName, PoweredOnEmissiveStrength);
            Material->SetScalarParameterValue(ScreenPowerParameterName, 1.0f);
        }
    }
}

void AInteractivePC::FindHUD()
{
    if (HUD) return;
    TArray<UUserWidget*> Widgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, Widgets, UGameHUDWidget::StaticClass(), false);
    if (!Widgets.IsEmpty()) HUD = Cast<UGameHUDWidget>(Widgets[0]);
}

void AInteractivePC::RefreshPrompt()
{
    if (!bPlayerNearby) return;
    FindHUD();
    if (!HUD) return;

    FText Text;
    if (!HasMansionPower()) Text = FText::FromString(TEXT("NO POWER - RESTORE MANSION POWER"));
    else if (BootState == EPCBootState::Booting) Text = FText::FromString(TEXT("PC IS BOOTING..."));
    else if (BootState == EPCBootState::On) Text = FText::FromString(TEXT("PC ONLINE"));
    else Text = FText::FromString(TEXT("Press E to Turn On PC"));
    HUD->ShowInteractionPromptWithText(Text);
}

void AInteractivePC::HidePrompt()
{
    FindHUD();
    if (HUD) HUD->HideInteractionPrompt();
}
