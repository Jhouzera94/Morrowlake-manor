// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"

// Sets default values
AInteractableComponent::AInteractableComponent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AInteractableComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInteractableComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AInteractableComponent::Interact()
{
    if (!bHasPower)
    {
        UE_LOG(LogTemp, Warning, TEXT("This object has no power."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Object interacted with!"));
}
