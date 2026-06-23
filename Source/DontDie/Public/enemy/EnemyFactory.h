// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyFactory.generated.h"
class ASurvivorActor;


UCLASS()
class DONTDIE_API AEnemyFactory : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyFactory();

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	class UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSubclassOf<class AEnemyActor> Enemy;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	TSubclassOf<ASurvivorActor> SurvivorClass;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings", meta = (ClampMin = "0", ClampMax = "100"))
	int32 SurvivorSpawnChance = 20;

	UPROPERTY(EditAnywhere, Category = "Spawn Settings")
	float DelayTime = 3.f;

private:
	float CurrentTime = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void SpawnEnemies(int32 Count);
};
