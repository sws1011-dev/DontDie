// Fill out your copyright notice in the Description page of Project Settings.


#include "DontDieGameModeBase.h"

#include "EnemyFactory.h"
#include "PlayerPawn.h"
#include "UpgradeWidget.h"
#include "Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "DontDieSaveGame.h"

void ADontDieGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	LoadTotalGold();

	if (TargetFactory == nullptr)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyFactory::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			TargetFactory = Cast<AEnemyFactory>(FoundActors[0]);
		}
	}

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
	bIsWaveEnding = false; // 새 웨이브 시작 시 플래그 초기화
	TimeElapsedInWave = 0.0f;

	// 10웨이브마다 시간과 스폰 수를 초기화하기 위해 상대적 웨이브 값 계산 (1~10 반복)
	int32 RelativeWave = ((CurrentWave - 1) % 10) + 1;

	WaveDuration = 5.0f + (RelativeWave * 5.0f);

	// 웨이브 공식: 상대적 웨이브에 따라 스폰 수 결정 (10, 15, 20... 55마리 후 다시 10마리)
	TotalEnemiesInWave = 5 + (RelativeWave * 5);
	RemainingEnemyToSpawn = TotalEnemiesInWave;

	CurrentAliveEnemyCount = 0;
	CurrentAliveSurvivorCount = 0;

	GetWorldTimerManager().SetTimer(WaveTimerHandle, this, &ADontDieGameModeBase::UpdateWaveTimer, 0.1f, true);
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ADontDieGameModeBase::SpawnZombieGroup, 2.0f, true);
}

void ADontDieGameModeBase::UpdateWaveTimer()
{
	if (bIsWaveEnding) return;

	TimeElapsedInWave += 0.1f;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
		if (Player) Player->RefreshHUD();
	}

	// 매 틱(0.1초)마다 종료 조건을 확인하여, 마지막 적이 죽은 후 즉시 반응하도록 합니다.
	CheckWaveEnd();

	if (TimeElapsedInWave >= WaveDuration)
	{
		if (GetWorldTimerManager().IsTimerActive(SpawnTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
			UE_LOG(LogTemp, Warning, TEXT("Wave Time Up! Stopping Spawner. Mop up remaining enemies!"));
		}

		CheckWaveEnd();
	}
}

void ADontDieGameModeBase::AddAliveEnemyCount(int32 Amount)
{
	CurrentAliveEnemyCount += Amount;
}

void ADontDieGameModeBase::AddAliveSurvivorCount(int32 Amount)
{
	CurrentAliveSurvivorCount += Amount;
}

void ADontDieGameModeBase::OnEnemyKilled()
{
	CurrentAliveEnemyCount = FMath::Max(0, CurrentAliveEnemyCount - 1);
	UE_LOG(LogTemp, Log, TEXT("Enemy Killed. Alive Count: %d"), CurrentAliveEnemyCount);

	CheckWaveEnd();
}

void ADontDieGameModeBase::OnEnemyOverlapDestroyed()
{
	CurrentAliveEnemyCount = FMath::Max(0, CurrentAliveEnemyCount - 1);
	UE_LOG(LogTemp, Warning, TEXT("Enemy Detonated on Player. Alive Count: %d"), CurrentAliveEnemyCount);

	CheckWaveEnd();
}

void ADontDieGameModeBase::OnSurvivorRemoved()
{
	CurrentAliveSurvivorCount = FMath::Max(0, CurrentAliveSurvivorCount - 1);
	UE_LOG(LogTemp, Log, TEXT("Survivor Removed. Alive Survivor Count: %d"), CurrentAliveSurvivorCount);

	CheckWaveEnd();
}

void ADontDieGameModeBase::CheckWaveEnd()
{
	// 이미 웨이브 종료 중이거나 타이머가 돌아가고 있다면 무시
	if (bIsWaveEnding || GetWorldTimerManager().IsTimerActive(WaveEndDelayTimerHandle)) return;

	if (CurrentAliveEnemyCount <= 0 && CurrentAliveSurvivorCount <= 0)
	{
		if (RemainingEnemyToSpawn <= 0 || TimeElapsedInWave >= WaveDuration)
		{
			// 즉시 종료하지 않고, 약간의 지연 시간을 두어 사운드나 UI 연출이 끝나게 함
			GetWorldTimerManager().SetTimer(WaveEndDelayTimerHandle, this, &ADontDieGameModeBase::EndWave, WaveEndDelay, false);
			UE_LOG(LogTemp, Warning, TEXT("Wave Conditions Met. Ending Wave in %f seconds..."), WaveEndDelay);
		}
	}
}

void ADontDieGameModeBase::EndWave()
{
	// 중복 실행 방지
	if (bIsWaveEnding) return;
	bIsWaveEnding = true;

	// 타이머 일시 정지
	GetWorldTimerManager().ClearTimer(WaveTimerHandle);
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

	UE_LOG(LogTemp, Warning, TEXT("Wave %d Cleared! Preparing Reward Selection..."), CurrentWave);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();

	if (UpgradeWidgetClass != nullptr)
	{
		UUpgradeWidget* UpgradeUI = CreateWidget<UUpgradeWidget>(GetWorld(), UpgradeWidgetClass);
		if (UpgradeUI != nullptr)
		{
			TArray<FUpgradeCardData> Options = GenerateUpgradeOptions();
			UpgradeUI->SetupUpgradeOptions(Options);
			UpgradeUI->AddToViewport();

			if (PC)
			{
				PC->SetShowMouseCursor(true);

				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(UpgradeUI->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
	}

	// 게임 일시정지는 맨 마지막에 가동합니다.
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ADontDieGameModeBase::MoveToNextWave()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// 다시 게임 플레이를 위해 마우스 커서는 유지하되(회전용), 게임 입력도 받도록 설정
		PC->SetShowMouseCursor(true);

		FInputModeGameAndUI InputMode;
		PC->SetInputMode(InputMode);

		UGameplayStatics::SetGamePaused(GetWorld(), false);
	}

	if (CurrentWave >= MaxWave)
	{
		FinalizeGold();
		return;
	}

	CurrentWave++;
	StartWave();
}

void ADontDieGameModeBase::FinalizeGold()
{
	TotalGold += CurrentGold;
	SaveTotalGold();

	UE_LOG(LogTemp, Warning, TEXT("Finalized Gold: %d added. New Total: %d"), CurrentGold, TotalGold);

	CurrentGold = 0;

	// UI 갱신을 위해 플레이어 HUD 새로고침 호출
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
		if (Player) Player->RefreshHUD();
	}
}

void ADontDieGameModeBase::LoadTotalGold()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UDontDieSaveGame* SaveGameInstance = Cast<
			UDontDieSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
		if (SaveGameInstance)
		{
			TotalGold = SaveGameInstance->TotalGold;
			UpgradeLevels = SaveGameInstance->UpgradeLevels;
		}
	}
	else
	{
		TotalGold = 0;
		UpgradeLevels.Empty();
	}
}

void ADontDieGameModeBase::SaveTotalGold()
{
	UDontDieSaveGame* SaveGameInstance = Cast<UDontDieSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UDontDieSaveGame::StaticClass()));
	if (SaveGameInstance)
	{
		SaveGameInstance->TotalGold = TotalGold;
		SaveGameInstance->UpgradeLevels = UpgradeLevels;
		UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveSlotName, 0);
	}
}

void ADontDieGameModeBase::ApplyPersistentUpgrades(APlayerPawn* Player)
{
	if (!Player) return;

	// 영구 이동 속도
	if (UpgradeLevels.Contains(TEXT("MoveSpeed")))
	{
		Player->MoveSpeed += UpgradeLevels[TEXT("MoveSpeed")] * 20.0f;
	}

	// 영구 최대 체력
	if (UpgradeLevels.Contains(TEXT("MaxHP")))
	{
		Player->MaxHP += UpgradeLevels[TEXT("MaxHP")] * 10.0f;
		Player->CurrentHP = Player->MaxHP;
	}

	// 영구 크리티컬 확률
	if (UpgradeLevels.Contains(TEXT("CritChance")))
	{
		Player->CritChance += UpgradeLevels[TEXT("CritChance")] * 0.02f;
	}

	// 영구 목숨 추가
	if (UpgradeLevels.Contains(TEXT("LifeCount")))
	{
		Player->CurrentLife += UpgradeLevels[TEXT("LifeCount")];
	}

	// 영구 골드 배율
	if (UpgradeLevels.Contains(TEXT("CurrencyMultiplier")))
	{
		Player->CurrencyMultiplier += UpgradeLevels[TEXT("CurrencyMultiplier")] * 0.05f;
	}

	if (Player->CurrentWeapon)
	{
		// 영구 데미지
		if (UpgradeLevels.Contains(TEXT("BaseDamage")))
		{
			Player->CurrentWeapon->BaseDamage += UpgradeLevels[TEXT("BaseDamage")] * 2.0f;
		}

		// 영구 연사 속도
		if (UpgradeLevels.Contains(TEXT("FireRate")))
		{
			Player->CurrentWeapon->FireRate += UpgradeLevels[TEXT("FireRate")] * 0.2f;
		}

		// 영구 최대 장탄수
		if (UpgradeLevels.Contains(TEXT("MaxAmmo")))
		{
			Player->CurrentWeapon->MaxAmmo += UpgradeLevels[TEXT("MaxAmmo")] * 2;
			Player->CurrentWeapon->CurrentAmmo = Player->CurrentWeapon->MaxAmmo;
		}
	}

	Player->RefreshHUD();
}

float ADontDieGameModeBase::GetWaveProgress() const
{
	if (WaveDuration <= 0.0f) return 0.0f;

	float Progress = TimeElapsedInWave / WaveDuration;

	return FMath::Clamp(Progress, 0.0f, 1.0f);
}

TArray<FUpgradeCardData> ADontDieGameModeBase::GenerateUpgradeOptions()
{
	TArray<FUpgradeCardData> SelectedOptions;
	TArray<EUpgradeType> AvailablePool;

	AvailablePool.Add(EUpgradeType::MoveSpeed);
	AvailablePool.Add(EUpgradeType::MaxHP);
	AvailablePool.Add(EUpgradeType::CritChance);
	AvailablePool.Add(EUpgradeType::ProjectileCount);
	AvailablePool.Add(EUpgradeType::LifeCount);
	AvailablePool.Add(EUpgradeType::CurrencyMultiplier);
	AvailablePool.Add(EUpgradeType::BaseDamage);
	AvailablePool.Add(EUpgradeType::FireRate);
	AvailablePool.Add(EUpgradeType::MaxAmmo);
	AvailablePool.Add(EUpgradeType::ReloadSpeed);

	while (SelectedOptions.Num() < 3 && AvailablePool.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, AvailablePool.Num() - 1);
		EUpgradeType PickedType = AvailablePool[RandomIndex];
		AvailablePool.RemoveAt(RandomIndex);

		FUpgradeCardData NewCard;
		NewCard.UpgradeType = PickedType;

		switch (PickedType)
		{
		case EUpgradeType::MoveSpeed:
			NewCard.DisplayName = TEXT("경량화 신발");
			NewCard.Description = TEXT("이동 속도가 50 증가합니다.");
			break;
		case EUpgradeType::MaxHP:
			NewCard.DisplayName = TEXT("강인한 체력");
			NewCard.Description = TEXT("최대 체력이 20 증가합니다.");
			break;
		case EUpgradeType::CritChance:
			NewCard.DisplayName = TEXT("치명적 약점");
			NewCard.Description = TEXT("크리티컬 확률이 5% 증가합니다.");
			break;
		case EUpgradeType::ProjectileCount:
			NewCard.DisplayName = TEXT("다탄두 장전");
			NewCard.Description = TEXT("발사되는 투사체의 개수가 1개 증가합니다.");
			break;
		case EUpgradeType::LifeCount:
			NewCard.DisplayName = TEXT("비상용 심장");
			NewCard.Description = TEXT("부활할 수 있는 목숨이 1개 추가됩니다.");
			break;
		case EUpgradeType::CurrencyMultiplier:
			NewCard.DisplayName = TEXT("황금 나침반");
			NewCard.Description = TEXT("재화 획득량이 15% 증가합니다.");
			break;
		case EUpgradeType::BaseDamage:
			NewCard.DisplayName = TEXT("강화 화약");
			NewCard.Description = TEXT("무기 기본 공격력이 5 증가합니다.");
			break;
		case EUpgradeType::FireRate:
			NewCard.DisplayName = TEXT("고속 연사");
			NewCard.Description = TEXT("무기의 공격 속도가 빨라집니다.");
			break;
		case EUpgradeType::MaxAmmo:
			NewCard.DisplayName = TEXT("대용량 탄창");
			NewCard.Description = TEXT("최대 장탄수가 4발 증가합니다.");
			break;
		case EUpgradeType::ReloadSpeed:
			NewCard.DisplayName = TEXT("전술 재장전");
			NewCard.Description = TEXT("재장전 시간이 단축됩니다.");
			break;
		}

		SelectedOptions.Add(NewCard);
	}

	return SelectedOptions;
}

void ADontDieGameModeBase::ApplyUpgrade(EUpgradeType ChosenUpgrade)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
	if (!Player)
	{
		return;
	}

	switch (ChosenUpgrade)
	{
	case EUpgradeType::MoveSpeed:
		Player->MoveSpeed += 50.f;
		break;
	case EUpgradeType::MaxHP:
		Player->MaxHP += 20.f;
		Player->CurrentHP = FMath::Clamp(Player->CurrentHP + 20.f, 0.f, Player->MaxHP);
		break;
	case EUpgradeType::CritChance:
		Player->CritChance = FMath::Min(1.0f, Player->CritChance + 0.05f);
		break;
	case EUpgradeType::ProjectileCount:
		Player->ProjectileCount += 1;
		break;
	case EUpgradeType::LifeCount:
		Player->CurrentLife += 1;
		break;
	case EUpgradeType::CurrencyMultiplier:
		Player->CurrencyMultiplier += 0.15f;
		break;

	case EUpgradeType::BaseDamage:
		if (Player->CurrentWeapon) Player->CurrentWeapon->BaseDamage += 5.f;
		break;
	case EUpgradeType::FireRate:
		if (Player->CurrentWeapon) Player->CurrentWeapon->FireRate += 0.5f;
		break;
	case EUpgradeType::MaxAmmo:
		if (Player->CurrentWeapon)
		{
			Player->CurrentWeapon->MaxAmmo += 4;
		}
		break;
	case EUpgradeType::ReloadSpeed:
		if (Player->CurrentWeapon)
			Player->CurrentWeapon->ReloadSpeed = FMath::Max(
				0.5f, Player->CurrentWeapon->ReloadSpeed - 0.3f);
		break;
	}

	Player->RefreshHUD();
	MoveToNextWave();
}

void ADontDieGameModeBase::AddGold(int32 Amount)
{
	if (Amount <= 0) return;

	CurrentGold += Amount;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		APlayerPawn* Player = Cast<APlayerPawn>(PC->GetPawn());
		if (Player) Player->RefreshHUD();
	}
}
