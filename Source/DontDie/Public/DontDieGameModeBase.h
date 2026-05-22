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
	MoveSpeed UMETA(DisplayName = "이동 속도 증가"),
	MaxHP UMETA(DisplayName = "최대 체력 증가"),
	CritChance UMETA(DisplayName = "크리티컬 확률 증가"),
	ProjectileCount UMETA(DisplayName = "투사체 개수 증가"),
	LifeCount UMETA(DisplayName = "목숨 추가"),
	CurrencyMultiplier UMETA(DisplayName = "재화 획득량 증가"),
	BaseDamage UMETA(DisplayName = "무기 데미지 증가"),
	FireRate UMETA(DisplayName = "공격 속도 증가"),
	MaxAmmo UMETA(DisplayName = "최대 장탄수 증가"),
	ReloadSpeed UMETA(DisplayName = "재장전 속도 감소"),
	MAX_COUNT UMETA(Hidden) // 총 개수 체크용
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

	// 웨이브가 5씩 진행될 때마다 적용될 좀비 스탯 증가 수치
	UPROPERTY(EditAnywhere, Category = "Wave System|Scaling")
	float HealthIncrementPer5Waves = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Wave System|Scaling")
	float AttackPowerIncrementPer5Waves = 5.0f;

	// 시간 기반 웨이브 변수들
	float WaveDuration; // 이번 웨이브의 총 제한 시간
	float TimeElapsedInWave; // 현재 경과 시간
	FTimerHandle WaveTimerHandle; // 웨이브 전체 시간(제한 시간)용 타이머

	int32 TotalEnemiesInWave;
	int32 RemainingEnemyToSpawn;
	int32 CurrentAliveEnemyCount;
	int32 CurrentAliveSurvivorCount = 0;

	FTimerHandle SpawnTimerHandle;

	UFUNCTION(BlueprintCallable, Category = "Wave System")
	TArray<FUpgradeCardData> GenerateUpgradeOptions();

	// UI에서 카드를 클릭했을 때 어떤 능력치인지 넘겨받아 실제 반영하는 함수
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

	// 내부 시간 갱신 함수
	void UpdateWaveTimer();
	void EndWave();

	bool bIsWaveEnding = false;

	// 모든 적이 처치되었는지 확인하고 웨이브를 종료하는 함수
	void CheckWaveEnd();

	UPROPERTY(EditAnywhere, Category = "Wave System")
	float WaveEndDelay = 1.5f;

	FTimerHandle WaveEndDelayTimerHandle;

	// 블루프린트 보상 UI에서 선택이 끝나면 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Wave System")
	void MoveToNextWave();

	UPROPERTY(EditDefaultsOnly, Category = "Wave System")
	class AEnemyFactory* TargetFactory;

	UFUNCTION(BlueprintPure, Category = "WaveSystem")
	float GetWaveProgress() const;

	// 플레이어 현재 보유 재화
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 CurrentGold = 0;

	// 영구 보관되는 전체 재화
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	int32 TotalGold = 0;

	// 영구 업그레이드 레벨 데이터
	UPROPERTY(BlueprintReadOnly, Category = "Economy")
	TMap<FString, int32> UpgradeLevels;

	// 민간인 탈출 성공 시 지급할 기본 보상 액수
	UPROPERTY(EditDefaultsOnly, Category = "Economy")
	int32 CivilianRescueReward = 100;

	// 재화 추가 함수 (민간인 탈출 or 좀비 처치 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddGold(int32 Amount);

	// 세션 종료 시 현재 골드를 전체 골드에 합산하는 함수
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void FinalizeGold();

	// 저장된 영구 업그레이드 수치를 플레이어에게 적용하는 함수
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void ApplyPersistentUpgrades(class APlayerPawn* Player);

private:
	void LoadTotalGold();
	void SaveTotalGold();
	const FString SaveSlotName = TEXT("DontDieSaveSlot");
};
