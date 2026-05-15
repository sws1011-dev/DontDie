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