// Fill out your copyright notice in the Description page of Project Settings.


#include "player/CombatPlayerCharacter.h"

#include "gamemode/DontDieGameModeBase.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "widget/GameOverWidget.h"
#include "widget/PlayerHudWidget.h"
#include "weapon/Weapon.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ACombatPlayerCharacter::ACombatPlayerCharacter()
{
}

void ACombatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentLifeCount = MaxLifeCount;

	if (PlayerController != nullptr)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsys =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());

		if (Subsys != nullptr && ImcCombat != nullptr)
		{
			Subsys->AddMappingContext(ImcCombat, 1);
		}
	}

	if (PlayerHudWidgetClass != nullptr)
	{
		HUDWidget = CreateWidget<UPlayerHudWidget>(GetWorld(), PlayerHudWidgetClass);
		if (HUDWidget != nullptr)
		{
			HUDWidget->AddToViewport();
			HUDWidget->UpdateLifeText(CurrentLifeCount);
			UpdateReloadingHUD(false);
			UE_LOG(LogTemp, Warning, TEXT("HUD Created and Added to Viewport!"));
		}
	}

	if (DefaultWeaponClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;

		CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass, GetActorLocation(), GetActorRotation(), SpawnParams);
		if (CurrentWeapon != nullptr)
		{
			const FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, true);
			CurrentWeapon->AttachToComponent(GetMesh(), AttachRules, TEXT("WeaponSocket"));

			UpdateAmmoHUD(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
		}
	}
}

void ACombatPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACombatPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	if (IaFire != nullptr)
	{
		EnhancedInputComponent->BindAction(IaFire, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Fire);
	}

	if (IaReload != nullptr)
	{
		EnhancedInputComponent->BindAction(IaReload, ETriggerEvent::Started, this, &ACombatPlayerCharacter::Reload);
	}
}

void ACombatPlayerCharacter::Fire()
{
	if (CurrentWeapon != nullptr)
	{
		float DamageMultiplier = 1.0f;
		if (FMath::FRand() < CritChance)
		{
			DamageMultiplier = 2.0f;
		}

		CurrentWeapon->Fire(ProjectileCount, DamageMultiplier);
	}
}

void ACombatPlayerCharacter::Reload()
{
	if (CurrentWeapon != nullptr)
	{
		CurrentWeapon->Reload();
	}
}

float ACombatPlayerCharacter::GetCalculatedDamage()
{
	if (CurrentWeapon != nullptr)
	{
		float FinalDamage = CurrentWeapon->BaseDamage;
		if (FMath::FRand() < CritChance)
		{
			FinalDamage *= 2.0f;
		}
		return FinalDamage;
	}

	return 0.0f;
}

void ACombatPlayerCharacter::OnReceiveDamage(float DamageAmount)
{
	CurrentHP = FMath::Clamp(CurrentHP - DamageAmount, 0.0f, MaxHP);
	RefreshHUD();

	if (CurrentHP <= 0.0f)
	{
		DecreaseLifeCount();
	}
}

void ACombatPlayerCharacter::DecreaseLifeCount()
{
	CurrentLifeCount = FMath::Max(0, CurrentLifeCount - 1);

	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateLifeText(CurrentLifeCount);
	}

	if (CurrentLifeCount > 0)
	{
		CurrentHP = MaxHP;
		RefreshHUD();
		UE_LOG(LogTemp, Warning, TEXT("Player life consumed. Remaining lives: %d"), CurrentLifeCount);
		return;
	}

	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	int32 EarnedGold = 0;
	int32 TotalGold = 0;
	if (GM != nullptr)
	{
		EarnedGold = GM->CurrentGold;
		GM->FinalizeGold();
		TotalGold = GM->TotalGold;
	}

	if (GameOverWidgetClass != nullptr)
	{
		UGameOverWidget* GameOverUI = CreateWidget<UGameOverWidget>(GetWorld(), GameOverWidgetClass);
		if (GameOverUI != nullptr)
		{
			GameOverUI->SetupResults(EarnedGold, TotalGold);
			GameOverUI->AddToViewport();

			APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
			if (PC != nullptr)
			{
				PC->SetShowMouseCursor(true);
				PC->SetInputMode(FInputModeUIOnly());
			}

			UGameplayStatics::SetGamePaused(GetWorld(), true);
		}
	}
}

void ACombatPlayerCharacter::RefreshHUD()
{
	ADontDieGameModeBase* GM = Cast<ADontDieGameModeBase>(GetWorld()->GetAuthGameMode());

	if (HUDWidget == nullptr)
	{
		return;
	}

	if (MaxHP > 0.0f)
	{
		const float HPPercent = CurrentHP / MaxHP;
		HUDWidget->UpdateHPBar(HPPercent);
	}

	if (GM != nullptr)
	{
		const float Progress = GM->GetWaveProgress();
		HUDWidget->UpdateWaveProgress(Progress);
		HUDWidget->UpdateWaveStageText(GM->CurrentWave);
		HUDWidget->UpdateGoldText(GM->CurrentGold);
	}

	HUDWidget->UpdateLifeText(CurrentLifeCount);

	if (CurrentWeapon != nullptr)
	{
		HUDWidget->UpdateAmmoText(CurrentWeapon->CurrentAmmo, CurrentWeapon->MaxAmmo);
	}
}

void ACombatPlayerCharacter::UpdateAmmoHUD(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->UpdateAmmoText(CurrentAmmo, MaxAmmo);
	}
}

void ACombatPlayerCharacter::UpdateReloadingHUD(bool bIsReloading)
{
	if (HUDWidget != nullptr)
	{
		HUDWidget->SetReloadingVisibility(bIsReloading);
	}
}
