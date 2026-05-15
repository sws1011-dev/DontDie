// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DontDieGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API ADontDieGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	int32 CurrentWave = 1;
	int32 MaxWave = 10;

	int32 TotalEnemiesInWave;
	int32 RemainingEnemyToSpawn;
	int32 CurrentAliveEnemyCount;

	FTimerHandle SpawnTimerHandle;

	void OnEnemyKilled();
	void AddAliveEnemyCount(int32 Amount);
	void StartWave();
	void SpawnZombieGroup();

	UPROPERTY(EditDefaultsOnly, Category = "Wave System")
	class AEnemyFactory* TargetFactory;

	UFUNCTION(BlueprintPure, Category = "WaveSystem")
	float GetWaveProgress() const;
};
