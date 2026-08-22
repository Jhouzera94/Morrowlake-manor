#include "SequentialDialogueTrigger.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerController.h"

ASequentialDialogueTrigger::ASequentialDialogueTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    RootComponent = TriggerBox;
    TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
    TriggerBox->SetBoxExtent(FVector(150.0f));

    DialogueAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudio"));
    DialogueAudio->SetupAttachment(TriggerBox);
    DialogueAudio->bAutoActivate = false;
}

void ASequentialDialogueTrigger::BeginPlay()
{
    Super::BeginPlay();
    TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ASequentialDialogueTrigger::OnPlayerEnter);
    DialogueAudio->OnAudioFinished.AddDynamic(this, &ASequentialDialogueTrigger::OnDialogueFinished);
}

void ASequentialDialogueTrigger::OnPlayerEnter(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || OtherActor != PC->GetPawn() || bDialoguePlaying || DialogueAudio->IsPlaying()) return;
    if (!DialogueLines.IsValidIndex(NextLineIndex) || !DialogueLines[NextLineIndex]) return;

    DialogueAudio->SetSound(DialogueLines[NextLineIndex]);
    bDialoguePlaying = true;
    DialogueAudio->Play();
}

void ASequentialDialogueTrigger::OnDialogueFinished()
{
    bDialoguePlaying = false;
    ++NextLineIndex;
    if (NextLineIndex >= DialogueLines.Num())
    {
        NextLineIndex = bLoopAfterFinalLine ? 0 : DialogueLines.Num();
    }
}
