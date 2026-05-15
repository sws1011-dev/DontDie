// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyFactory.generated.h"

UCLASS()
class DONTDIE_API AEnemyFactory : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AEnemyFactory();

	UPROPERTY(EditAnywhere)
	class UBoxComponent* SpawnArea;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AEnemyActor> Enemy;

	UPROPERTY(EditAnywhere)
	float DelayTime = 3.f;

private:
	float CurrentTime = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void SpawnEnemies(int32 Count);
};
