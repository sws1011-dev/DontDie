// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHudWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPlayerHudWidget::UpdateLifeText(int32 CurrentLife)
{
	if (LifeText != nullptr)
	{
		FString LifeString = FString::Printf(TEXT("x %d"), CurrentLife);
		LifeText->SetText(FText::FromString(LifeString));
	}
}

void UPlayerHudWidget::UpdateWaveProgress(float Percent)
{
	if (WaveProgressBar)
	{
		WaveProgressBar->SetPercent(Percent);
	}
}

void UPlayerHudWidget::UpdateAmmoText(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoText != nullptr)
	{
		// "12 / 12" 형태로 문자열 포맷팅
		FString AmmoString = FString::Printf(TEXT("%d / %d"), CurrentAmmo, MaxAmmo);

		// 텍스트 블록에 반영
		AmmoText->SetText(FText::FromString(AmmoString));
	}
}

void UPlayerHudWidget::SetReloadingVisibility(bool bIsReloading)
{
	if (ReloadText != nullptr)
	{
		if (bIsReloading)
		{
			// 재장전 중이면 텍스트를 화면에 표시
			ReloadText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			// 재장전이 끝나면 텍스트를 완전히 숨김 (공간 차지 안 함)
			ReloadText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}
