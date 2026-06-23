// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHudWidget.generated.h"

/**
 * 
 */
UCLASS()
class DONTDIE_API UPlayerHudWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta =(BindWidget))
	class UTextBlock* LifeText;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HPProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* WaveProgressBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WaveStageText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ReloadText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* GoldText;

	void UpdateAmmoText(int32 CurrentAmmo, int32 MaxAmmo);
	void UpdateLifeText(int32 CurrentLife);
	void UpdateHPBar(float Percent);
	void UpdateWaveProgress(float Percent);
	void SetReloadingVisibility(bool bIsReloading);
	void UpdateGoldText(int32 CurrentGold);
	void UpdateWaveStageText(int32 CurrentWave);
};
