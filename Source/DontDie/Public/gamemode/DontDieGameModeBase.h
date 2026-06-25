// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DontDieGameModeBase.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EUpgradeType : uint8
{
	MoveSpeed UMETA(DisplayName = "?´ë™ ?ë„ ì¦ê?"),
	MaxHP UMETA(DisplayName = "ìµœë? ì²´ë ¥ ì¦ê?"),
	CritChance UMETA(DisplayName = "?¬ë¦¬?°ì»¬ ?•ë¥  ì¦ê?"),
	ProjectileCount UMETA(DisplayName = "?¬ì‚¬ì²?ê°œìˆ˜ ì¦ê?"),
	LifeCount UMETA(DisplayName = "ëª©ìˆ¨ ì¶”ê?"),
	CurrencyMultiplier UMETA(DisplayName = "?¬í™” ?ë“??ì¦ê?"),
	BaseDamage UMETA(DisplayName = "ë¬´ê¸° ?°ë?ì§€ ì¦ê?"),
	FireRate UMETA(DisplayName = "ê³µê²© ?ë„ ì¦ê?"),
	MaxAmmo UMETA(DisplayName = "ìµœë? ?¥íƒ„??ì¦ê?"),
	ReloadSpeed UMETA(DisplayName = "?¬ì¥???ë„ ê°ì†Œ"),
	MAX_COUNT UMETA(Hidden) // ì´?ê°œìˆ˜ ì²´í¬??
};

USTRUCT(BlueprintType)
struct FUpgradeCardData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	EUpgradeType UpgradeType;

	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Upgrade")
	FString Description;
};

UCLASS()
class DONTDIE_API ADontDieGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	int32 CurrentWave = 1;
	int32 MaxWave = 100;

	// ?¨ì´ë¸Œê? 5??ì§„í–‰???Œë§ˆ???ìš©??ì¢€ë¹??¤íƒ¯ ì¦ê? ?˜ì¹˜
	UPROPERTY(EditAnywhere, Category = "Wave System|Scaling")
	float HealthIncrementPer5Waves = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Wave System|Scaling")
	float AttackPowerIncrementPer5Waves = 5.0f;

	// ?œê°„ ê¸°ë°˜ ?¨ì´ë¸?ë³€?˜ë“¤
	float WaveDuration; // ?´ë²ˆ ?¨ì´ë¸Œì˜ ì´??œí•œ ?œê°„
	float TimeElapsedInWave; // ?„ì¬ ê²½ê³¼ ?œê°„
	FTimerHandle WaveTimerHandle; // ?¨ì´ë¸??„ì²´ ?œê°„(?œí•œ ?œê°„)???€?´ë¨¸

	int32 TotalEnemiesInWave;
	int32 RemainingEnemyToSpawn;
	int32 CurrentAliveEnemyCount;
	int32 CurrentAliveSurvivorCount = 0;

	FTimerHandle SpawnTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Wave System")
	TArray<FUpgradeCardData> GenerateUpgradeOptions();

	// UI?ì„œ ì¹´ë“œë¥??´ë¦­?ˆì„ ???´ë–¤ ?¥ë ¥ì¹˜ì¸ì§€ ?˜ê²¨ë°›ì•„ ?¤ì œ ë°˜ì˜?˜ëŠ” ?¨ìˆ˜
	UFUNCTION(BlueprintCallable, Category = "Wave System")
	void ApplyUpgrade(EUpgradeType ChosenUpgrade);

	UPROPERTY(EditDefaultsOnly, Category = "Wave System|UI")
	TSubclassOf<class UUpgradeWidget> UpgradeWidgetClass;

	void OnEnemyKilled();
	void OnSurvivorRemoved();
	void OnEnemyOverlapDestroyed();
	void AddAliveEnemyCount(int32 Amount);
	void AddAliveSurvivorCount(int32 Amount);
	void StartWave();
	void SpawnZombieGroup();

	// ?´ë? ?œê°„ ê°±ì‹  ?¨ìˆ˜
	void UpdateWaveTimer();
	void EndWave();

	bool bIsWaveEnding = false;

	// ëª¨ë“  ?ì´ ì²˜ì¹˜?˜ì—ˆ?”ì? ?•ì¸?˜ê³  ?¨ì´ë¸Œë? ì¢…ë£Œ?˜ëŠ” ?¨ìˆ˜
	void CheckWaveEnd();

	UPROPERTY(EditAnywhere, Category = "Wave System")
	float WaveEndDelay = 1.5f;

	FTimerHandle WaveEndDelayTimerHandle;

	// ë¸”ë£¨?„ë¦°??ë³´ìƒ UI?ì„œ ? íƒ???ë‚˜ë©??¸ì¶œ???¨ìˆ˜
	UFUNCTION(BlueprintCallable, Category = "Wave System")
	void MoveToNextWave();

	UPROPERTY(EditDefaultsOnly, Category = "Wave System")
	class AEnemyFactory* TargetFactory;

	UFUNCTION(BlueprintPure, Category = "WaveSystem")
	float GetWaveProgress() const;

	// ?Œë ˆ?´ì–´ ?„ì¬ ë³´ìœ  ?¬í™”
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 CurrentGold = 0;

	// ?êµ¬ ë³´ê??˜ëŠ” ?„ì²´ ?¬í™”
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TotalGold = 0;

	// ?êµ¬ ?…ê·¸?ˆì´???ˆë²¨ ?°ì´??
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<FString, int32> UpgradeLevels;

	// ë¯¼ê°„???ˆì¶œ ?±ê³µ ??ì§€ê¸‰í•  ê¸°ë³¸ ë³´ìƒ ?¡ìˆ˜
	UPROPERTY(EditDefaultsOnly, Category = "Economy")
	int32 CivilianRescueReward = 100;

	// ?¬í™” ì¶”ê? ?¨ìˆ˜ (ë¯¼ê°„???ˆì¶œ or ì¢€ë¹?ì²˜ì¹˜ ???¸ì¶œ)
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddGold(int32 Amount);

	// ?¸ì…˜ ì¢…ë£Œ ???„ì¬ ê³¨ë“œë¥??„ì²´ ê³¨ë“œ???©ì‚°?˜ëŠ” ?¨ìˆ˜
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void FinalizeGold();

	// ?€?¥ëœ ?êµ¬ ?…ê·¸?ˆì´???˜ì¹˜ë¥??Œë ˆ?´ì–´?ê²Œ ?ìš©?˜ëŠ” ?¨ìˆ˜
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ApplyPersistentUpgrades(class ACombatPlayerCharacter* Player);

private:
	void LoadTotalGold();
	void SaveTotalGold();
	const FString SaveSlotName = TEXT("DontDieSaveSlot");
};
