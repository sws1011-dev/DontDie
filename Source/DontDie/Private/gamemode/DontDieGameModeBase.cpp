// Fill out your copyright notice in the Description page of Project Settings.


#include "gamemode/DontDieGameModeBase.h"

#include "enemy/EnemyFactory.h"
#include "widget/UpgradeWidget.h"
#include "weapon/Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

#include "data/DontDieSaveGame.h"
#include "player/CombatPlayerCharacter.h"

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
			ACombatPlayerCharacter* Player = Cast<ACombatPlayerCharacter>(PC->GetPawn());
			if (Player)
			{
				Player->RefreshHUD();
			}
		}
	}
}

void ADontDieGameModeBase::StartWave()
{
	bIsWaveEnding = false; // ???⑥씠釉??쒖옉 ???뚮옒洹?珥덇린??
	TimeElapsedInWave = 0.0f;

	// 10?⑥씠釉뚮쭏???쒓컙怨??ㅽ룿 ?섎? 珥덇린?뷀븯湲??꾪빐 ?곷????⑥씠釉?媛?怨꾩궛 (1~10 諛섎났)
	int32 RelativeWave = ((CurrentWave - 1) % 10) + 1;

	WaveDuration = 5.0f + (RelativeWave * 5.0f);

	// ?⑥씠釉?怨듭떇: ?곷????⑥씠釉뚯뿉 ?곕씪 ?ㅽ룿 ??寃곗젙 (10, 15, 20... 55留덈━ ???ㅼ떆 10留덈━)
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
		ACombatPlayerCharacter* Player = Cast<ACombatPlayerCharacter>(PC->GetPawn());
		if (Player) Player->RefreshHUD();
	}

	// 留???0.1珥?留덈떎 醫낅즺 議곌굔???뺤씤?섏뿬, 留덉?留??곸씠 二쎌? ??利됱떆 諛섏쓳?섎룄濡??⑸땲??
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
	// ?대? ?⑥씠釉?醫낅즺 以묒씠嫄곕굹 ??대㉧媛 ?뚯븘媛怨??덈떎硫?臾댁떆
	if (bIsWaveEnding || GetWorldTimerManager().IsTimerActive(WaveEndDelayTimerHandle)) return;

	if (CurrentAliveEnemyCount <= 0 && CurrentAliveSurvivorCount <= 0)
	{
		if (RemainingEnemyToSpawn <= 0 || TimeElapsedInWave >= WaveDuration)
		{
			// 利됱떆 醫낅즺?섏? ?딄퀬, ?쎄컙??吏???쒓컙???먯뼱 ?ъ슫?쒕굹 UI ?곗텧???앸굹寃???
			GetWorldTimerManager().SetTimer(WaveEndDelayTimerHandle, this, &ADontDieGameModeBase::EndWave, WaveEndDelay, false);
			UE_LOG(LogTemp, Warning, TEXT("Wave Conditions Met. Ending Wave in %f seconds..."), WaveEndDelay);
		}
	}
}

void ADontDieGameModeBase::EndWave()
{
	// 以묐났 ?ㅽ뻾 諛⑹?
	if (bIsWaveEnding) return;
	bIsWaveEnding = true;

	// ??대㉧ ?쇱떆 ?뺤?
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

	// 寃뚯엫 ?쇱떆?뺤???留?留덉?留됱뿉 媛?숉빀?덈떎.
	UGameplayStatics::SetGamePaused(GetWorld(), true);
}

void ADontDieGameModeBase::MoveToNextWave()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		// ?ㅼ떆 寃뚯엫 ?뚮젅?대? ?꾪빐 留덉슦??而ㅼ꽌???좎??섎릺(?뚯쟾??, 寃뚯엫 ?낅젰??諛쏅룄濡??ㅼ젙
		PC->SetShowMouseCursor(false);

		FInputModeGameOnly InputMode;
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

	// UI 媛깆떊???꾪빐 ?뚮젅?댁뼱 HUD ?덈줈怨좎묠 ?몄텧
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		ACombatPlayerCharacter* Player = Cast<ACombatPlayerCharacter>(PC->GetPawn());
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

void ADontDieGameModeBase::ApplyPersistentUpgrades(ACombatPlayerCharacter* Player)
{
	if (!Player) return;

	// ?곴뎄 ?대룞 ?띾룄
	if (UpgradeLevels.Contains(TEXT("MoveSpeed")))
	{
		Player->MoveSpeed += UpgradeLevels[TEXT("MoveSpeed")] * 20.0f;
	}

	// ?곴뎄 理쒕? 泥대젰
	if (UpgradeLevels.Contains(TEXT("MaxHP")))
	{
		Player->MaxHP += UpgradeLevels[TEXT("MaxHP")] * 10.0f;
		Player->CurrentHP = Player->MaxHP;
	}

	// ?곴뎄 ?щ━?곗뺄 ?뺣쪧
	if (UpgradeLevels.Contains(TEXT("CritChance")))
	{
		Player->CritChance += UpgradeLevels[TEXT("CritChance")] * 0.02f;
	}

	// ?곴뎄 紐⑹닲 異붽?
	if (UpgradeLevels.Contains(TEXT("LifeCount")))
	{
		Player->CurrentLifeCount += UpgradeLevels[TEXT("LifeCount")];
	}

	// ?곴뎄 怨⑤뱶 諛곗쑉
	if (UpgradeLevels.Contains(TEXT("CurrencyMultiplier")))
	{
		Player->CurrencyMultiplier += UpgradeLevels[TEXT("CurrencyMultiplier")] * 0.05f;
	}

	if (Player->CurrentWeapon)
	{
		// ?곴뎄 ?곕?吏
		if (UpgradeLevels.Contains(TEXT("BaseDamage")))
		{
			Player->CurrentWeapon->BaseDamage += UpgradeLevels[TEXT("BaseDamage")] * 2.0f;
		}

		// ?곴뎄 ?곗궗 ?띾룄
		if (UpgradeLevels.Contains(TEXT("FireRate")))
		{
			Player->CurrentWeapon->FireRate += UpgradeLevels[TEXT("FireRate")] * 0.2f;
		}

		// ?곴뎄 理쒕? ?ν깂??
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
			NewCard.DisplayName = TEXT("Move Speed");
			NewCard.Description = TEXT("Move speed increases by 50.");
			break;
		case EUpgradeType::MaxHP:
			NewCard.DisplayName = TEXT("Max HP");
			NewCard.Description = TEXT("Max HP increases by 20.");
			break;
		case EUpgradeType::CritChance:
			NewCard.DisplayName = TEXT("Critical Chance");
			NewCard.Description = TEXT("Critical chance increases by 5%.");
			break;
		case EUpgradeType::ProjectileCount:
			NewCard.DisplayName = TEXT("Projectile Count");
			NewCard.Description = TEXT("Projectile count increases by 1.");
			break;
		case EUpgradeType::LifeCount:
			NewCard.DisplayName = TEXT("Extra Life");
			NewCard.Description = TEXT("Life count increases by 1.");
			break;
		case EUpgradeType::CurrencyMultiplier:
			NewCard.DisplayName = TEXT("Currency Multiplier");
			NewCard.Description = TEXT("Currency gain increases by 15%.");
			break;
		case EUpgradeType::BaseDamage:
			NewCard.DisplayName = TEXT("Base Damage");
			NewCard.Description = TEXT("Weapon base damage increases by 5.");
			break;
		case EUpgradeType::FireRate:
			NewCard.DisplayName = TEXT("Fire Rate");
			NewCard.Description = TEXT("Weapon fire rate increases.");
			break;
		case EUpgradeType::MaxAmmo:
			NewCard.DisplayName = TEXT("Max Ammo");
			NewCard.Description = TEXT("Max ammo increases by 4.");
			break;
		case EUpgradeType::ReloadSpeed:
			NewCard.DisplayName = TEXT("Reload Speed");
			NewCard.Description = TEXT("Reload time decreases.");
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

	ACombatPlayerCharacter* Player = Cast<ACombatPlayerCharacter>(PC->GetPawn());
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
		Player->CurrentLifeCount += 1;
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
		ACombatPlayerCharacter* Player = Cast<ACombatPlayerCharacter>(PC->GetPawn());
		if (Player) Player->RefreshHUD();
	}
}
