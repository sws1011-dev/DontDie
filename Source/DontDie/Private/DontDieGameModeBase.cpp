// Fill out your copyright notice in the Description page of Project Settings.


#include "DontDieGameModeBase.h"

#include "EnemyFactory.h"
#include "PlayerPawn.h"
#include "Kismet/GameplayStatics.h"

void ADontDieGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// 만약 에디터에서 TargetFactory를 지정하지 않았다면 자동으로 찾아오기
	if (TargetFactory == nullptr)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyFactory::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			TargetFactory = Cast<AEnemyFactory>(FoundActors[0]);
		}
	}

	// 첫 번째 웨이브 시작
	StartWave();
}

void ADontDieGameModeBase::SpawnZombieGroup()
{
	if (RemainingEnemyToSpawn <= 0)
	{
		GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
		return;
	}

	if (TargetFactory != nullptr)
	{
		int32 SpawnCount = FMath::Min(1, RemainingEnemyToSpawn);

		TargetFactory->SpawnEnemies(SpawnCount);
		RemainingEnemyToSpawn -= SpawnCount;

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
			if (Player)
			{
				Player->RefreshHUD();
			}
		}
	}
}

void ADontDieGameModeBase::StartWave()
{
	// 웨이브 공식: 웨이브마다 5마리씩 증가 (10, 15, 20...)
	TotalEnemiesInWave = 5 + (CurrentWave * 5);
	RemainingEnemyToSpawn = TotalEnemiesInWave;
	CurrentAliveEnemyCount = 0;

	UE_LOG(LogTemp, Warning, TEXT("=== Wave %d Started! Target: %d ==="), CurrentWave, TotalEnemiesInWave);

	// 2초마다 SpawnZombieGroup 함수 호출 (반복)
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ADontDieGameModeBase::SpawnZombieGroup, 2.0f, true);
}

void ADontDieGameModeBase::AddAliveEnemyCount(int32 Amount)
{
	CurrentAliveEnemyCount += Amount;
}

void ADontDieGameModeBase::OnEnemyKilled()
{
	CurrentAliveEnemyCount--;

	UE_LOG(LogTemp, Log, TEXT("Enemy Killed. Remaining: %d, Alive: %d"), RemainingEnemyToSpawn, CurrentAliveEnemyCount);

	// 스폰도 끝났고, 맵에 남은 좀비도 없다면 다음 웨이브!
	if (RemainingEnemyToSpawn <= 0 && CurrentAliveEnemyCount <= 0)
	{
		if (CurrentWave < MaxWave)
		{
			CurrentWave++;
			UE_LOG(LogTemp, Warning, TEXT("Wave Cleared! Next Wave in 3 seconds..."));

			// 3초 뒤에 다음 웨이브 시작 (타이머 재사용)
			GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ADontDieGameModeBase::StartWave, 3.0f, false);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("All Waves Cleared! YOU WIN!"));
			// 여기에 승리 UI 띄우는 로직 추가 가능
		}
	}
}

float ADontDieGameModeBase::GetWaveProgress() const
{
	// 1. 안전 장치: 분모가 0이면 0 리턴
	if (TotalEnemiesInWave <= 0) return 0.0f;

	// 2. 현재까지 처치한 수 계산
	int32 KilledEnemies = TotalEnemiesInWave - (RemainingEnemyToSpawn + CurrentAliveEnemyCount);

	// 3. 실수 연산을 위해 float로 강제 형변환 (매우 중요!)
	// (float)를 안 붙이면 1/10 같은 연산 결과가 0.1이 아니라 0이 됩니다.
	float Progress = (float)KilledEnemies / (float)TotalEnemiesInWave;

	// 4. 로그를 찍어서 소수점이 나오는지 확인해봅시다.
	// UE_LOG(LogTemp, Warning, TEXT("Killed: %d / Total: %d / Result: %f"), KilledEnemies, TotalEnemiesInWave, Progress);

	return FMath::Clamp(Progress, 0.0f, 1.0f);
}
