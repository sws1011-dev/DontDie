// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHpWidget.h"

#include "Components/ProgressBar.h"

void UEnemyHpWidget::UpdateHealthBar(float HealthPercent)
{
	if (HpBar != nullptr)
	{
		HpBar->SetPercent(HealthPercent);
	}
}
