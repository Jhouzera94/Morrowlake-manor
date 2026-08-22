// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

// Sets default values
AInventoryComponent::AInventoryComponent()
{
	// Inventory state changes only through explicit actions.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AInventoryComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

