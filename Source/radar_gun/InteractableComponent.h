// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableComponent.generated.h"

UCLASS()
class RADAR_GUN_API AInteractableComponent : public AActor
{
	GENERATED_BODY()

    public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Power")
	bool bHasPower = false;

	UFUNCTION(BlueprintCallable)
	void Interact();

	// Sets default values for this actor's properties
	AInteractableComponent();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
